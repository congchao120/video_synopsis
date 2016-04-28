#include "Ctracker.h"
#include "synopsis.h"
using namespace cv;
using namespace std;

size_t CTrack::NextTrackID=0;
// ---------------------------------------------------------------------------
// Track constructor.
// The track begins from initial point (pt)
// ---------------------------------------------------------------------------
CTrack::CTrack(Point2f pt, float dt, float Accel_noise_mag, int n_tube_index)
{
	track_id=NextTrackID;

	NextTrackID++;
	// Every track have its own Kalman filter,
	// it user for next point position prediction.
	KF = new TKalmanFilter(pt,dt,Accel_noise_mag);
	// Here stored points coordinates, used for next position prediction.
	prediction=pt;
	skipped_frames=0;
	n_tube_map_index = n_tube_index;
}
// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
CTrack::~CTrack()
{
	// Free resources.
	delete KF;
}

// ---------------------------------------------------------------------------
// Tracker. Manage tracks. Create, remove, update.
// ---------------------------------------------------------------------------
CTracker::CTracker(float _dt, float _Accel_noise_mag, vector< list<simple_tube *> > & database, double _dist_thres, int _maximum_allowed_skipped_frames,int _max_trace_length)
{
dt=_dt;
Accel_noise_mag=_Accel_noise_mag;
dist_thres=_dist_thres;
maximum_allowed_skipped_frames=_maximum_allowed_skipped_frames;
max_trace_length=_max_trace_length;
p_tube_database = &database;
n_tube_index = 0;
}
// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CTracker::Update(vector<Point2d>& detections, vector<CvRect>& detect_rects, int n_frame)
{

	if(detections.size() != detect_rects.size())
	{
		printf("Error in update!\n");
	}
	// -----------------------------------
	// If there is no tracks yet, then every point begins its own track.
	// -----------------------------------
	if(tracks.size()==0)
	{
		// If no tracks yet
		for(int i=0;i<detections.size();i++)
		{
			CTrack* tr=new CTrack(detections[i],dt,Accel_noise_mag, n_tube_index);
			tracks.push_back(tr);
			simple_tube* s_tube = new simple_tube(detect_rects[i], n_frame);
			p_tube_database->push_back(list<simple_tube *>(1, s_tube));
			n_tube_index++;
		}	
	}

	

	// -----------------------------------
	// «дес?трек?уж?есть ?любо?случае
	// -----------------------------------
	int N=tracks.size();		// трек?	
	int M=detections.size();	// детект?
	// ћатриц?рассто€ни?от N-ного трек?до M-ного детект?
	vector< vector<double> > Cost(N,vector<double>(M));
	vector<int> assignment; // назначен?

	// -----------------------------------
	// “рек?уж?есть, составим матриц?рассто€ни?	// -----------------------------------
	double dist;
	for(int i=0;i<tracks.size();i++)
	{	
		// Point2d prediction=tracks[i]->prediction;
		// cout << prediction << endl;
		for(int j=0;j<detections.size();j++)
		{
			Point2d diff=(tracks[i]->prediction-detections[j]);
			dist=sqrtf(diff.x*diff.x+diff.y*diff.y);
			Cost[i][j]=dist;
		}
	}
	// -----------------------------------
	// Solving assignment problem (tracks and predictions of Kalman filter)
	// -----------------------------------
	AssignmentProblemSolver APS;
	APS.Solve(Cost,assignment,AssignmentProblemSolver::optimal);

	// -----------------------------------
	// clean assignment from pairs with large distance
	// -----------------------------------
	// Not assigned tracks
	vector<int> not_assigned_tracks;

	for(int i=0;i<assignment.size();i++)
	{
		if(assignment[i]!=-1)
		{
			if(Cost[i][assignment[i]]>dist_thres)
			{
				assignment[i]=-1;
				// Mark unassigned tracks, and increment skipped frames counter,
				// when skipped frames counter will be larger than threshold, track will be deleted.
				not_assigned_tracks.push_back(i);
			}
		}
		else
		{			
			// If track have no assigned detect, then increment skipped frames counter.
			tracks[i]->skipped_frames++;
		}

	}

	// -----------------------------------
	// If track didn't get detects long time, remove it.
	// -----------------------------------
	for(int i=0;i<tracks.size();i++)
	{
		if(tracks[i]->skipped_frames>maximum_allowed_skipped_frames)
		{
			delete tracks[i];
			tracks.erase(tracks.begin()+i);
			assignment.erase(assignment.begin()+i);
			i--;
		}
	}
	// -----------------------------------
	// Search for unassigned detects
	// -----------------------------------
	vector<int> not_assigned_detections;
	vector<int>::iterator it;
	for(int i=0;i<detections.size();i++)
	{
		it=find(assignment.begin(), assignment.end(), i);
		if(it==assignment.end())
		{
			not_assigned_detections.push_back(i);
		}
	}

	// -----------------------------------
	// and start new tracks for them.
	// -----------------------------------
	if(not_assigned_detections.size()!=0)
	{
		for(int i=0;i<not_assigned_detections.size();i++)
		{
			CTrack* tr=new CTrack(detections[not_assigned_detections[i]],dt,Accel_noise_mag, n_tube_index);
			tracks.push_back(tr);

			simple_tube* s_tube = new simple_tube(detect_rects[not_assigned_detections[i]], n_frame);
			p_tube_database->push_back(list<simple_tube *>(1, s_tube));
			n_tube_index++;
		}	
	}

	// Update Kalman Filters state

	for(int i=0;i<assignment.size();i++)
	{
		// If track updated less than one time, than filter state is not correct.

		tracks[i]->KF->GetPrediction();

		if(assignment[i]!=-1) // If we have assigned detect, then update using its coordinates,
		{
			tracks[i]->skipped_frames=0;
			tracks[i]->prediction=tracks[i]->KF->Update(detections[assignment[i]],1);
			//update tube
			simple_tube* s_tube = new simple_tube(detect_rects[assignment[i]], n_frame);
			int i_tube = tracks[i]->n_tube_map_index;
			(*p_tube_database)[i_tube].push_back(s_tube);
		}else				  // if not continue using predictions
		{
			tracks[i]->prediction=tracks[i]->KF->Update(Point2f(0,0),0);	
		}
		
		if(tracks[i]->trace.size()>max_trace_length)
		{
			tracks[i]->trace.erase(tracks[i]->trace.begin(),tracks[i]->trace.end()-max_trace_length);
		}

		tracks[i]->trace.push_back(tracks[i]->prediction);
		tracks[i]->KF->LastResult=tracks[i]->prediction;
	}

}
// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
CTracker::~CTracker(void)
{
	for(int i=0;i<tracks.size();i++)
	{
	delete tracks[i];
	}
	tracks.clear();
}
