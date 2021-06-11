/*
  Dataset.h - Library for maintaining a dataset of integers
  Created by Robin Gray, June 7, 2021.
*/

#ifndef Dataset_h
#define Dataset_h

#include "Arduino.h"

#ifndef DATASET_SIZE
  #define DATASET_SIZE 3    // The size of the dataset
#endif

#define MAX_BYTE 255        // Maximum number stored in a byte

class Dataset {
  private:
    unsigned int _data[DATASET_SIZE];
    byte _count;
    unsigned int _average;
  public:
    void clear();
    unsigned int average();
    void add(unsigned int value);
};

#endif
