/*
  Dataset.cpp - Library for maintaining a dataset of integers
  Created by Robin Gray, June 7, 2021.
*/

#include "Arduino.h"
#include "Dataset.h"

void Dataset::clear() {
  _count = 0;
  _average = 0;
  memset(this->_data, 0, sizeof(this->_data));
}

int Dataset::average() {
  return this->_average;
}

void Dataset::add(int value) {
  unsigned long sum;
  byte i;
  sum = 0;
  // Store the value in the array
  _data[_count % DATASET_SIZE] = value;

  // Update the average
  for (i = 0; i < DATASET_SIZE; i++)
  {
    sum += _data[i];

    if(i >= _count)
      break;
  }

  if (_count == 0)
    _average = (unsigned int)sum;
  else if(_count < DATASET_SIZE)
    _average = (unsigned int)sum/_count;
  else
    _average = (unsigned int)sum/DATASET_SIZE;
    
  
  // Increment the counter
  if(_count == MAX_BYTE)
    _count = DATASET_SIZE;
  else
    _count++;
}
