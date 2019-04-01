#pragma once

#include <base/copyable.h>
#include <base/Types.h>

#include <boost/operators.hpp>

//实现微秒级的Timestamp
class Timestamp : public copyable,
                  public boost::equality_comparable<Timestamp>,
                  public boost::less_than_comparable<Timestamp>{
 public:
  Timestamp(): microSecsSinceEpoch_(0) {}	//构造一个无效的Timestamp

  explicit Timestamp(int64_t microSecsSinceEpoch)
	  : microSecsSinceEpoch_(microSecsSinceEpoch)
  {}

  void swap(Timestamp& that){
    std::swap(microSecsSinceEpoch_, that.microSecsSinceEpoch_);
  }

  string toString() const;
  string toFormattedString() const;

  bool valid() const { return microSecsSinceEpoch_ > 0; }


  int64_t microSecsSinceEpoch() const { return microSecsSinceEpoch_; }
  time_t secsSinceEpoch() const
  { return static_cast<time_t>(microSecsSinceEpoch_ / MicroSecsPerSec); }


  static Timestamp now();
  static Timestamp invalid(){ return Timestamp();}

  static Timestamp fromUnixTime(time_t t){ return fromUnixTime(t, 0);}

  static Timestamp fromUnixTime(time_t t, int microseconds){
    return Timestamp(static_cast<int64_t>(t) * MicroSecsPerSec + microseconds);
  }

  static const int MicroSecsPerSec = 1000 * 1000;

 private:
  int64_t microSecsSinceEpoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs){
  return lhs.microSecsSinceEpoch() < rhs.microSecsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs){
  return lhs.microSecsSinceEpoch() == rhs.microSecsSinceEpoch();
}

//返回值的单位是秒
inline double timeDifference(Timestamp high, Timestamp low){
  int64_t diff = high.microSecsSinceEpoch() - low.microSecsSinceEpoch();
  return static_cast<double>(diff) / Timestamp::MicroSecsPerSec;
}

inline Timestamp addTime(Timestamp timestamp, double seconds){
  int64_t delta = static_cast<int64_t>(seconds * Timestamp::MicroSecsPerSec);
  return Timestamp(timestamp.microSecsSinceEpoch() + delta);
}
