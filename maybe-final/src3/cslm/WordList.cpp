/*
 * This file is part of the continuous space language and translation model toolkit
 * for statistical machine translation and large vocabulary speech recognition.
 *
 * Copyright 2014, Holger Schwenk, LIUM, University of Le Mans, France
 *
 * The CSLM toolkit is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * $Id: WordList.cpp,v 1.6 2014/03/25 21:52:53 schwenk Exp $
 */

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include "WordList.h"

// initialize constant values and special tokens
const WordList::WordIndex WordList::BadIndex = (NULL_WORD - 1); ///< bad word list index
const char *WordList::WordUnknown   = "<unk>"; ///< unknown word token
const char *WordList::WordSentStart = "<s>"  ; ///< sent start token
const char *WordList::WordSentEnd   = "</s>" ; ///< sent end token
const char *WordList::WordPause     = "-pau-"; ///< pause token

/**
 * constructs void word list
 * @param use_tokens use special tokens 'unknown word', 'sent start', 'sent end' and 'pause' (default no)
 */
WordList::WordList(bool use_tokens) :
    slist_len(0),
    use_tokens(use_tokens),
    frequ_sort(false)
{
  // fill special tokens
  if (use_tokens) {
    specTokens.push_back(WordList::WordUnknown);
    specTokens.push_back(WordList::WordSentStart);
    specTokens.push_back(WordList::WordSentEnd);
    specTokens.push_back(WordList::WordPause);
    InsertTokens();
  }
}

/**
 * destroys word list
 */
WordList::~WordList()
{
  boost::unordered_map<const char*,WordInfo,WordMapHash,WordMapPred>::const_iterator iter, end = wordHash.cend();
  for (iter = wordHash.cbegin(); iter != end ; iter++)
    if (NULL != iter->second.word)
      delete[] iter->second.word;
}

/**
 * compares two word list elements
 * @param p first element
 * @param q second element
 * @return true if word frequency of first element is more than the second one
 */
bool wlist_comp(const WordList::WordInfo *p, const WordList::WordInfo *q) {
  return (p->n > q->n);
}

/**
 * reads word list from file
 * @note if lines of file contains a count after word, the word list will be sorted by word frequency (most frequent to least frequent)
 * @param fname file name
 * @param slist_len length of short list (default -1 for all words in file)
 * @return number of words read (not the size of word list, depending of special tokens and duplicates)
 */
WordList::WordIndex WordList::Read(const char *fname, WordList::WordIndex slist_len)
{
  wlist.clear();
  sortMap.clear();
  wordHash.clear();

  // open word list file
  std::ifstream dfs_wlst;
  dfs_wlst.open(fname, std::ios::in);
  CHECK_FILE(dfs_wlst, fname);

  // skip blank lines
  std::stringbuf sb;
  std::size_t n_lines = 0;
  do {
    dfs_wlst.get(sb);
    dfs_wlst.clear();
    n_lines++;
    std::istringstream iss(sb.str());
    iss >> std::ws;
    if (iss.good())
      break;
    else
      dfs_wlst.get();
  } while (dfs_wlst.good());

  // check format
  unsigned short n_fields = 0;
  std::string str;
  if (!dfs_wlst)
    Error("can't read first line of word list");
  else {
    std::istringstream iss(sb.str());
    int val;
    iss >> str;
    while ((!iss.fail()) && (n_fields <= 3)) {
      n_fields++;
      iss >> val;
    }
    switch (n_fields) {
      case 1:
        frequ_sort = false;
        break;
      case 2:
      case 3:
        frequ_sort = true;
        break;
      default:
        ErrorN("found %u fields in first line of word list", n_fields);
    }
  }

  // insert special tokens
  WordIndex new_index = InsertTokens();

  // read word list
  WordIndex n_words_read = 0;
  int word_count, word_class;
  do {
    std::string sb_str = sb.str();
    std::istringstream iss(sb_str);
    iss >> std::ws;
    if (iss.good() && ((!use_tokens) || (sb_str.compare(0, 2, "##") != 0))) { // skip blank lines and lines starting by ##
      // read word
      if (n_fields >= 1)
        iss >> str;

      // read count
      if (n_fields >= 2)
        iss >> word_count;
      else
        word_count = 0;

      // read class
      if (n_fields >= 3)
        iss >> word_class;
      else
        word_class = 0;

      // get stream status
      if (iss.fail()) {
        ErrorN("There seems to be a format error in line %zd of word list", n_lines);
      }

      // check if word is already in hash table
      const char *cstr = str.c_str();
      if (wordHash.find(cstr) == wordHash.cend()) {
        WordIndex tmp_index = (new_index + 1);
        if (tmp_index > new_index) {
          // insert word
          char *new_word = strdup(cstr);
          WordInfo &new_w_info = wordHash[new_word];
          new_w_info.word = new_word;
          new_w_info.id = new_index;
          new_w_info.n = word_count;
          new_w_info.cl = word_class;
          wlist.push_back(&new_w_info);
          new_index = tmp_index;
        }
        else
          break;
      }
      else {
        // update word
        WordInfo &new_w_info = wordHash[cstr];
        new_w_info.n = word_count;
        new_w_info.cl = word_class;
      }
      n_words_read++;
    }
    sb.str(std::string());

    // read next line
    dfs_wlst.clear();
    dfs_wlst.get();
    dfs_wlst.get(sb);
    n_lines++;
  } while ((sb.in_avail() > 0) || ((dfs_wlst.rdstate() & (std::ios::eofbit | std::ios::badbit)) == 0));
  dfs_wlst.close();

#if 0
  // duplicate last word
  if (n_words_read > 0) {
    WordIndex tmp_index = (new_index + 1);
    if (tmp_index > new_index) {
      // insert word
      WordInfo *last_w_info = wlist.back();
      str = last_w_info->word;
      str += ' ';
      WordInfo &new_w_info = wordHash[str.c_str()];
      new_w_info.word = strdup(last_w_info->word);
      new_w_info.id = last_w_info->id;
      new_w_info.n = last_w_info->n;
      new_w_info.cl = last_w_info->cl;
      wlist.push_back(&new_w_info);
      new_index = tmp_index;
      n_words_read++;
    }
  }
#endif

  // sort word list by word frequency
  if (frequ_sort) {
    std::sort(wlist.begin(), wlist.end(), wlist_comp);

    // set up mapping from wlist indices
    sortMap.reserve(new_index);
    sortMap.resize(new_index);
    std::vector<WordInfo*>::const_iterator iter = wlist.begin(), end = wlist.end();
    for (WordIndex id = 0 ; iter != end ; iter++, id++)
      sortMap[(*iter)->id] = id;
  }

  // set short list length
  this->slist_len = ((slist_len != (WordIndex)-1) ? slist_len : new_index);

  return n_words_read;
}

/**
 * adds a word
 * @note increment word count if already inserted
 * @param word word to insert
 * @return index of word (or WordList::BadIndex in case of overflow)
 */
WordList::WordIndex WordList::AddWord(const char *word)
{
  WordIndex new_index = (WordIndex)wlist.size();

  // verify word
  boost::unordered_map<const char*,WordInfo,WordMapHash,WordMapPred>::iterator iter = wordHash.find(word);
  if (iter != wordHash.end()) {
    // word already inserted
    WordInfo &w_info = iter->second;
    w_info.n++;
    return w_info.id;
  }
  else {
    // insert word
    WordIndex tmp_index = (new_index + 1);
    if (tmp_index > new_index) {
      char *new_word = strdup(word);
      WordInfo &new_w_info = wordHash[new_word];
      new_w_info.word = new_word;
      new_w_info.id = new_index;
      new_w_info.n = 1;
      new_w_info.cl = 0;
      wlist.push_back(&new_w_info);
    }
  }
  return new_index;
}

/**
 * removes a word
 * @param word word to delete
 */
void WordList::RemoveWord(const char *word)
{
  boost::unordered_map<const char*,WordInfo,WordMapHash,WordMapPred>::iterator iter = wordHash.find(word);
  if (iter != wordHash.end()) {
    WordIndex index = iter->second.id;
    wlist[frequ_sort ? sortMap[index] : index] = NULL;
    wordHash.erase(iter);
  }
}

/**
 * writes word list into file
 * @note each line of file will contain a word, a count and eventually a class number
 * @param fname file name
 * @param n_fields number of fields to write: 0/1: word only; 2: word and count; 3: word, count and class
 * @return number of words with count more than zero
 */
WordList::WordIndex WordList::Write(const char *fname, unsigned short n_fields) const
{
  WordIndex ndiff = 0;

  // open word list file
  std::ofstream dfs_wlst;
  dfs_wlst.open(fname, std::ios::out);
  CHECK_FILE(dfs_wlst, fname);

  // write word list
  std::vector<WordInfo*>::const_iterator iter, end = wlist.end();
  for (iter = wlist.begin() ; iter != end ; iter++) {
    WordInfo *wi = *iter;
    if (wi != NULL) {
      if (wi->n > 0)
        ndiff++;
      dfs_wlst << ((wi->word != NULL) ? wi->word : "(null)");
      if (n_fields >= 2) {
        dfs_wlst << ' ' << wi->n;
        if (n_fields >= 3)
          dfs_wlst << ' ' << wi->cl;
      }
      dfs_wlst << '\n';
    }
  }
  dfs_wlst.close();
  return ndiff;
}

/**
 * prints statistics on classes
 * @return number of classes
 */
int WordList::ClassStats() const
{
  const int disp_nb_classes = 10;
  int n = 0, c;
  std::vector<WordInfo*>::const_iterator iter, end = wlist.end();

  std::cout << " - statistics on classes in word list: ";

  // count classes
  for (iter = wlist.begin() ; iter != end; iter++) {
    WordInfo *wi = *iter;
    if ((wi != NULL) && (wi->cl > n))
      n = wi->cl;
  }
  n++; // account for class 0
  std::cout << n << " classes found" << std::endl;

  // histogram
  int cnts[n];
  for (c = 0 ; c < n ; c++)
    cnts[c] = 0;
  for (iter = wlist.begin() ; iter != end; iter++) {
    WordInfo *wi = *iter;
    if (wi != NULL)
      cnts[wi->cl]++;
  }
  std::cout << "   counts:";
  for (c = 0 ; (c < disp_nb_classes) && (c < n) ; c++)
    std::cout << " " << cnts[c];
  if (c >= disp_nb_classes)
    std::cout << " ... " << cnts[n-1];
  std::cout << std::endl;
  int cnt_min = cnts[0], cnt_max = cnts[0], cnt_avr = cnts[0];
  for (c = 1 ; c < n; c++) {
    if (cnts[c] < cnt_min)
      cnt_min = cnts[c];
    if (cnts[c] > cnt_max)
      cnt_max = cnts[c];
    cnt_avr += cnts[c];
  }
  std::cout.precision(2);
  std::cout << "   min=" << cnt_min << ", max=" << cnt_max << ", avr=" << ((float)cnt_avr) / n << std::endl;

  return n;
}

/**
 * gets sum of word counts
 * @param sum_slist out sum of counts in short list
 * @param sum_total out sum of counts in complete list
 */
void WordList::CountWords(int &sum_slist, int &sum_total) const
{
  std::vector<WordInfo*>::const_iterator iter = wlist.begin(), end = wlist.end();
  WordIndex ci = slist_len;
  for (sum_slist = 0 ; (iter != end) && (ci > 0) ; iter++, ci--)
    if ((*iter) != NULL)
      sum_slist += (*iter)->n;
  for (sum_total = sum_slist ; iter != end ; iter++)
    if ((*iter) != NULL)
      sum_total += (*iter)->n;
}

/**
 * equality function for hash table
 * @param w1 a word
 * @param w2 other word
 * @return true if two words are the same (case sensitive)
 */
bool WordList::WordMapPred::operator()(const char *w1, const char *w2) const
{
  return (strcmp(w1, w2) == 0);
}

/**
 * hash function for hash table
 * @param w a word
 * @return hash value
 */
std::size_t WordList::WordMapHash::operator()(const char *w) const
{
  std::size_t seed = 0;
  for (; (*w) != '\0' ; w++)
    boost::hash_combine(seed, *w);
  return seed;
}

/**
 * inserts special tokens into void word list
 * @return next available index in word list
 */
WordList::WordIndex WordList::InsertTokens()
{
  WordIndex new_index = (WordIndex)wlist.size();
  if (use_tokens && (new_index <= 0)) {
    // word list is void: insert tokens
    std::vector<const char*>::const_iterator iter, end = specTokens.end();
    for (iter = specTokens.begin() ; iter != end ; iter++) {
      char *new_word = strdup(*iter);
      WordInfo &new_w_info = wordHash[new_word];
      new_w_info.word = new_word;
      new_w_info.id = new_index++;
      new_w_info.n = 0;
      new_w_info.cl = 0;
      wlist.push_back(&new_w_info);
    }
  }
  return new_index;
}
