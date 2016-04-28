#pragma once
#include "Kalman.h"
#include "HungarianAlg.h"
#include "synopsis.h"
#include "opencv2/opencv.hpp"
#include <iostream>
#include <vector>
using namespace cv;
using namespace std;

class CTrack
{
public:
	vector<Point2d> trace;

	static size_t NextTrackID;
	size_t track_id;
	size_t skipped_frames; 
	Point2d prediction;
	TKalmanFilter* KF;
	int n_tube_map_index;
	CTrack(Point2f p, float dt, float Accel_noise_mag, int n_tube_index);
	~CTrack();
};


class CTracker
{
public:
	
	float dt; 

	float Accel_noise_mag;
	double dist_thres;

	int maximum_allowed_skipped_frames;

	int max_trace_length;

	int n_tube_index;

	vector<CTrack*> tracks;
	void Update(vector<Point2d>& detections, vector<CvRect>& detect_rects, int n_frame);
	CTracker(float _dt, float _Accel_noise_mag, vector< list<simple_tube *> > & database, double _dist_thres=60, int _maximum_allowed_skipped_frames=10,int _max_trace_length=10);
	~CTracker(void);
	vector< list<simple_tube *> > *p_tube_database;
};

