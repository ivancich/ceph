// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

/*
 * THIS FILE WILL LIKELY BE INTEGRATED IN Throttle.h. HOWEVER, SINCE
 * Throttle.h IS WIDELY INCLUDED, ANY CHANGES CAUSE LOTS OF
 * RECOMPILATION. SO FOR THE TIME BEING, WE'RE SEPARATING THIS OUT TO
 * REDUCE COMPILATION TIMES.
 */

#pragma once

#include <ostream>
#include "Mutex.h"
#include "common/Clock.h"


template<typename T>
class DataCollectionThrottle {
  struct ThrottleTiming {
    utime_t start;
    uint64_t count;
    uint64_t total;

    ThrottleTiming(uint64_t count, uint64_t total) :
      start(ceph_clock_now(NULL)),
      count(count),
      total(total)
    { }
  };

  std::string name;
  std::map<T,ThrottleTiming> map;
  uint64_t total;
  Mutex lock;

public:

  DataCollectionThrottle(const std::string& name) :
    name(name),
    total(0),
    lock(name)
  { }

  DataCollectionThrottle(const char* name) :
    name(name),
    total(0),
    lock(name)
  { }

  uint64_t get_current() {
    return total;
  }

  void get(uint64_t count, T index) {
    Mutex::Locker l(lock);
    total += count;
    map.emplace(index, ThrottleTiming(count, total));
  }

  ostream& put(T index, ostream& out) {
    utime_t duration;
    uint64_t count;
    uint64_t total_at_get;
    {
      Mutex::Locker l(lock);
      auto it = map.find(index);
      // assert(it != map.end())
      count = it->second.count;
      total -= count;
      total_at_get = it->second.total;
      duration = ceph_clock_now(NULL) - it->second.start;
      map.erase(it);
    }
    out << name << ": (" << total_at_get << "," << duration.to_nsec() << ")";
    return out;
  }
}; // class DataCollectionThrottle
