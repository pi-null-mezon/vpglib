/*
 * Copyright (c) 2015, Taranov Alex <pi-null-mezon@yandex.ru>.
 * Released to public domain under terms of the BSD Simplified license.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the organization nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *   See <http://www.opensource.org/licenses/bsd-license>
 */
#include "peakdetector.h"

namespace vpg {

PeakDetector::PeakDetector(int _signallength, int _intervalslength, int _intervalssubsetvolume, double _dT_ms)
{
    __init(_signallength, _intervalslength, _intervalssubsetvolume, _dT_ms);
}

PeakDetector::~PeakDetector()
{
    delete[] v_S;
    delete[] v_BS;
    delete[] v_T;
    delete[] v_DS;
    delete[] v_Intervals;
}

void PeakDetector::update(double value, double time)
{
    // Memorize signal count
    v_S[curposforsignal] = value;
    v_T[curposforsignal] = time;

    // Evaluate derivative with smooth
    v_DS[curposforsignal] = ( (v_S[curposforsignal] - v_S[__loop(curposforsignal-1)]) + v_DS[__loop(curposforsignal-1)] ) / 2.0;

    // Check if derivative has crossed zero, 4 counts is used for noise protection
    if(v_DS[curposforsignal] > 0.0 && v_DS[__loop(curposforsignal-1)] > 0.0 && v_DS[__loop(curposforsignal-3)] < 0.0 && v_DS[__loop(curposforsignal-4)] < 0.0) {
        // Mimimun has been found
        v_BS[__loop(curposforsignal-2)] = -1.0;

    } else if(v_DS[curposforsignal] < 0.0 && v_DS[__loop(curposforsignal-1)] < 0.0 && v_DS[__loop(curposforsignal-3)] > 0.0 && v_DS[__loop(curposforsignal-4)] > 0.0) {
        // Maximum has been found
        v_BS[__loop(curposforsignal-2)] = 1.0;

    } else {
        // No extremum has been found
        v_BS[__loop(curposforsignal-2)] = v_BS[__loop(curposforsignal-3)];
    }


    if(v_BS[__loop(curposforsignal-2)] == 1 && v_BS[__loop(curposforsignal-3)] == -1) {

        __updateInterval( __getDuration(lastfrontposition, __loop(curposforsignal - 2)));
        lastfrontposition = __loop(curposforsignal - 2);
    }

    curposforsignal = (curposforsignal + 1) % m_signallength;
}

const double *PeakDetector::getIntervalsVector() const
{
    return v_Intervals;
}

const double *PeakDetector::getBinarySignal() const
{
    return v_BS;
}

int PeakDetector::getSignalLength() const
{
    return m_signallength;
}

int PeakDetector::getIntervalsLength() const
{
    return m_intervalslength;
}

double PeakDetector::getCurrentInterval() const
{
    return v_Intervals[__seek(curposforinterval-1)];
}

int PeakDetector::getIntervalsPosition() const
{
    return curposforinterval;
}

double PeakDetector::averageCardiointervalms(int _n) const
{
    if(_n == 0) {
        return 0.0;
    }

    if(_n < 0) {
        _n = getIntervalsLength();
    }
    double _tms = 0.0;
    int _startpos = curposforinterval - 1;
    for(int i = 0; i < _n; ++i) {
        _tms += v_Intervals[__seek(_startpos - i)];
    }
    return _tms / _n;
}

void PeakDetector::__init(int _signallength, int _intervalslength, int _intervalssubsetvolume, double _dT_ms)
{
    curposforsignal = 0;
    curposforinterval = 0;
    lastfrontposition = 0;
    m_intervalssubsetvolume = _intervalssubsetvolume;

    m_signallength = _signallength;
    m_intervalslength = _intervalslength;

    v_S = new double[m_signallength];
    v_T = new double[m_signallength];
    v_DS = new double[m_signallength];
    v_BS = new double[m_signallength];
    for(int i = 0; i < m_signallength; i++) {
        v_S[i] = 0.0;
        v_T[i] = _dT_ms;
        v_DS[i] = 0.0;
        v_BS[i] = 0;
    }

    v_Intervals = new double[m_intervalslength];
    for(int i = 0; i < m_intervalslength; i++)
        v_Intervals[i] = i % 2 ? 200.0 : 1000.0;
}

void PeakDetector::__updateInterval(double _duration)
{
    double _mean = 0.0;
    for(int i = 0; i < m_intervalssubsetvolume; i++) {
        _mean += v_Intervals[__seek(curposforinterval - 1 - i)];
    }
    _mean /= m_intervalssubsetvolume;
    double _sko = 0.0;
    int _pos;
    for(int i = 0; i < m_intervalssubsetvolume; i++) {
        _pos = __seek(curposforinterval - 1 - i);
        _sko += (v_Intervals[_pos] - _mean)*(v_Intervals[_pos] - _mean);
    }
    _sko = std::sqrt( _sko/(m_intervalssubsetvolume - 1) );

    if( std::abs(_duration - _mean) > (3.0 * _sko) ) {
        //std::cout << "mean wins (m " << _mean << ", s " << _sko << ")" << std::endl;
        return;
    } else {
        v_Intervals[curposforinterval] = _duration;
        //std::cout << "duration " << _duration << "(m " << _mean << ", s " << _sko << ")"<<  std::endl;
        curposforinterval = (curposforinterval + 1) % m_intervalslength;
    }    
}

double PeakDetector::__getDuration(int start, int stop)
{
    double _duration = 0.0;
    int steps = (stop > start) ? (stop - start) : (stop + m_signallength - start);
    for(int i = 0; i < steps; i++)
        _duration += v_T[__loop(stop - i)];
    return _duration;
}

} // end of namespace vpg
