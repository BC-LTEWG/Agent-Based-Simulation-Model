#pragma once

#include <cmath>

const int DAY = 24;
const int WEEK = DAY * 7;
const int MONTH = DAY * 30;
const int YEAR = DAY * 365;

const double PRODUCTIVITY_THRESHOLD = 60.0; // threshold for productivity before lowering working hours
const int INITIAL_WORK_HOURS_DAILY = 8;

const double RANDOM_RETIREMENT_CHANCE = 8.3e-7;
const int GUARANTEED_RETIREMENT_AGE = 50 * YEAR; 
const double AVERAGE_RETIREMENT_AGE = (1 - pow(1 - RANDOM_RETIREMENT_CHANCE, GUARANTEED_RETIREMENT_AGE)) / RANDOM_RETIREMENT_CHANCE; // approx 45 years in hours

const double PRODUCT_FREQUENCY_MIN = 1.0 / (2 * WEEK);
const double PRODUCT_FREQUENCY_MAX = 1;

const double PERSON_FREQUENCY_MULTIPLIER_MEAN = 1.0;
const double PERSON_FREQUENCY_MULTIPLIER_STDDEV = 0.1;

const int STARTING_PEOPLE = 1000;
const int STARTING_PRODUCTS = 100;
const int STARTING_PRODUCERS = STARTING_PRODUCTS;
const int STARTING_DISTRIBUTORS = STARTING_PRODUCTS;
