// Implementation of the KMeans Algorithm
// reference: http://mnemstudio.org/clustering-k-means-example-1.htm
// reference: 

#include <iostream>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <chrono>
#include <tbb/task.h>
#include <tbb/tbb.h>
#include "cancelrange.h"
#include "ssedis.h"
using namespace std;

class Point
{
private:
	int id_point, id_cluster;
	vector<double> values;
	int total_values; //dimention of values
	string name;

public:
	Point(int id_point, vector<double>& values, string name = "")
	{
		this->id_point = id_point;
		total_values = values.size();

		for(int i = 0; i < total_values; i++)
			this->values.push_back(values[i]);

		this->name = name;
		id_cluster = -1;
	}

	int getID()
	{
		return id_point;
	}
	int getID() const 
	{
		return id_point;
	}
	void setCluster(int id_cluster)
	{
		this->id_cluster = id_cluster;
	}

	int getCluster()
	{
		return id_cluster;
	}

	double getValue(int index)
	{
		return values[index];
	}
	double getValue(int index) const
	{
		return values[index];
	}
	vector<double> getValue()
	{
		return values;
	}
	int getTotalValues()
	{
		return total_values;
	}

	void addValue(double value)
	{
		values.push_back(value);
	}

	string getName()
	{
		return name;
	}
};

class Cluster
{
private:
	int id_cluster;
	vector<double> central_values;
	vector<Point> points;

public:
	Cluster(int id_cluster, Point point)
	{
		this->id_cluster = id_cluster;

		int total_values = point.getTotalValues();

		for(int i = 0; i < total_values; i++)
			central_values.push_back(point.getValue(i));

		points.push_back(point);
	}

	void addPoint(Point point)
	{
		points.push_back(point);
	}

	bool removePoint(int id_point)
	{
		tbb::atomic<int> indexSearch;
		int total_points = points.size();
		
		
		
		//parallel_for version
		indexSearch = -1;
		//version 1		
		tbb::parallel_for(0,total_points, [=,&indexSearch](int i){				
			if(points[i].getID() == id_point)
			{				
				indexSearch = i;		

				tbb::task::self().cancel_group_execution();			
			}
		});
		/*
		//version 2
		bool stop = false;
		tbb::parallel_for(cancelable_range<int>(0,total_points,1,stop), searchBody<Point>(points,id_point));	
		*/	
		//v1 v2
		if (indexSearch == -1)
			return false;
		//return false;
		points.erase(points.begin() + indexSearch);
		return true;
		
		
		//parallel_reduce version
		
		/*int index = 
		tbb::parallel_reduce(tbb::blocked_range<int>(0, total_points), 
		-1, 
		[=](const tbb::blocked_range<int>& r, int index)->int{
			for(int i = r.begin(); i != r.end(); i++) {
				if(points[i].getID() == id_point){
					index = i;
					tbb::task::self().cancel_group_execution();
					return index;
				}					
			}
			return index;
		},
		[](int i, int j) -> int {
			return i == -1 ? j : i;
		}
		);

		if (index == -1)
			return false;
		points.erase(points.begin() + index);
		return true;*/
		/*
		for(int i = 0; i < total_points; i++)
		{
			if(points[i].getID() == id_point)
			{
				points.erase(points.begin() + i);
				return true;
			}
		}
		return false;*/
		
	}

	double getCentralValue(int index)
	{
		return central_values[index];
	}
	vector<double> getCentralValue() {
		return central_values;
	}
	void setCentralValue(int index, double value)
	{
		central_values[index] = value;
	}

	Point getPoint(int index)
	{
		return points[index];
	}

	int getTotalPoints()
	{
		return points.size();
	}

	int getID()
	{
		return id_cluster;
	}
};

class KMeans
{
private:
	int K; // number of clusters
	int total_values, total_points, max_iterations;
	vector<Cluster> clusters;

	// return ID of nearest center (uses euclidean distance)
	int getIDNearestCenter(Point point)
	{
		double sum = 0.0, min_dist;
		int id_cluster_center = 0;
		
		for(int i = 0; i < total_values; i++)
		{
			sum += pow(clusters[0].getCentralValue(i) -
					   point.getValue(i), 2.0);
		}
		//SSE
		//sum = distance(clusters[0].getCentralValue(), point.getValue());


		min_dist = sqrt(sum);

		for(int i = 1; i < K; i++)
		{
			double dist;
			sum = 0.0;
			
			for(int j = 0; j < total_values; j++)
			{
				sum += pow(clusters[i].getCentralValue(j) -
						   point.getValue(j), 2.0);
			}
			//sse
			//sum = distance(clusters[i].getCentralValue(), point.getValue());
			dist = sqrt(sum);

			if(dist < min_dist)
			{
				min_dist = dist;
				id_cluster_center = i;
			}
		}

		return id_cluster_center;
		
		//parallel_reduce
		/*return 
		tbb::parallel_reduce(tbb::blocked_range<int>(0,K),
		make_pair(100000000000,0),
		[=](const tbb::blocked_range<int>& r, pair<double,int> temp) -> pair<double,int> {
			for(int i = r.begin();i < r.end(); i++) {
				
				double dist;
				double sum = 0;
				//SSE
				for(int j = 0; j < total_values; j++)
				{
					sum += pow(clusters[i].getCentralValue(j) -
							point.getValue(j), 2.0);
				}

				dist = sqrt(sum);

				if(dist < temp.first)
				{
					temp.first = dist;
					temp.second = i;
				}				
			}
			return make_pair(temp.first, temp.second);
		},
		[](pair<double, int> a, pair<double, int> b) -> pair<double,int>{
			return a.first > b.first ? b : a;
		}
		).second;*/
	}

public:
	KMeans(int K, int total_points, int total_values, int max_iterations)
	{
		this->K = K;
		this->total_points = total_points;
		this->total_values = total_values;
		this->max_iterations = max_iterations;
	}

	void run(vector<Point> & points)
	{
        auto begin = chrono::high_resolution_clock::now();
        
		if(K > total_points)
			return;

		vector<int> prohibited_indexes;

		// choose K distinct values for the centers of the clusters
		
		
		for(int i = 0; i < K; i++)
		{
			/*
			while(true)
			{
				int index_point = rand() % total_points;

				if(find(prohibited_indexes.begin(), prohibited_indexes.end(),
						index_point) == prohibited_indexes.end())
				{
					prohibited_indexes.push_back(index_point);
					points[index_point].setCluster(i);
					Cluster cluster(i, points[index_point]);
					clusters.push_back(cluster);
					break;
				}
			}*/
			//set 0-K nodes as initial nodes for test
			points[i].setCluster(i);
			Cluster cluster(i, points[i]);
			clusters.push_back(cluster);
		}
        auto end_phase1 = chrono::high_resolution_clock::now(); 
        
		int iter = 1;

		while(true)
		{
			bool done = true;

			// associates each point to the nearest center
			

			for(int i = 0; i < total_points; i++)
			{
				int id_old_cluster = points[i].getCluster();
				int id_nearest_center = getIDNearestCenter(points[i]);

				if(id_old_cluster != id_nearest_center)
				{
					if(id_old_cluster != -1)
						clusters[id_old_cluster].removePoint(points[i].getID());

					points[i].setCluster(id_nearest_center);
					clusters[id_nearest_center].addPoint(points[i]);
					done = false;
				}
			}
			//parallel_for error
			/*tbb::parallel_for(0, total_points,[&](int i){
				int id_old_cluster = points[i].getCluster();
				int id_nearest_center = getIDNearestCenter(points[i]);

				if(id_old_cluster != id_nearest_center)
				{
					if(id_old_cluster != -1)
						clusters[id_old_cluster].removePoint(points[i].getID());

					points[i].setCluster(id_nearest_center);
					clusters[id_nearest_center].addPoint(points[i]);
					done = false;
				}				
			});*/
			// recalculating the center of each cluster

			for(int i = 0; i < K; i++)
			{
				for(int j = 0; j < total_values; j++)
				{
					int total_points_cluster = clusters[i].getTotalPoints();
					double sum = 0.0;

					if(total_points_cluster > 0)
					{
						
						for(int p = 0; p < total_points_cluster; p++)
							sum += clusters[i].getPoint(p).getValue(j);
						clusters[i].setCentralValue(j, sum / total_points_cluster);
					}
				}
			}
			//parallel_for (nested)
			/*tbb::parallel_for(0, K, [&](int i){
				tbb::parallel_for(0, total_values, [&](int j) {
					int total_points_cluster = clusters[i].getTotalPoints();
					double sum = 0.0;

					if(total_points_cluster > 0)
					{
						//sse not suitable
						for(int p = 0; p < total_points_cluster; p++)
							sum += clusters[i].getPoint(p).getValue(j);
						clusters[i].setCentralValue(j, sum / total_points_cluster);
					}
				});
			});*/
			//parallel_for (K)
			/*tbb::parallel_for(0, K, [&](int i){
				for(int j = 0; j < total_values; j++)
				{
					int total_points_cluster = clusters[i].getTotalPoints();
					double sum = 0.0;

					if(total_points_cluster > 0)
					{
						
						for(int p = 0; p < total_points_cluster; p++)
							sum += clusters[i].getPoint(p).getValue(j);
						clusters[i].setCentralValue(j, sum / total_points_cluster);
					}
				}
			});	*/	
			

			
			if(done == true || iter >= max_iterations)
			{
				cout << "Break in iteration " << iter << "\n\n";
				break;
			}

			iter++;
		}
        auto end = chrono::high_resolution_clock::now();
		cout << "TIME PHASE PER ITER = "  <<std::chrono::duration_cast<std::chrono::microseconds>(end-end_phase1).count()/iter<<"\n";
		// shows elements of clusters
		/*for(int i = 0; i < K; i++)
		{
			int total_points_cluster =  clusters[i].getTotalPoints();

			cout << "Cluster " << clusters[i].getID() + 1 << endl;
			for(int j = 0; j < total_points_cluster; j++)
			{
				cout << "Point " << clusters[i].getPoint(j).getID() + 1 << ": ";
				for(int p = 0; p < total_values; p++)
					cout << clusters[i].getPoint(j).getValue(p) << " ";

				string point_name = clusters[i].getPoint(j).getName();

				if(point_name != "")
					cout << "- " << point_name;

				cout << endl;
			}

			cout << "Cluster values: ";

			for(int j = 0; j < total_values; j++)
				cout << clusters[i].getCentralValue(j) << " ";

			cout << "\n\n";
            cout << "TOTAL EXECUTION TIME = "<<std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count()<<"\n";
            
            cout << "TIME PHASE 1 = "<<std::chrono::duration_cast<std::chrono::microseconds>(end_phase1-begin).count()<<"\n";
            
            cout << "TIME PHASE 2 = "<<std::chrono::duration_cast<std::chrono::microseconds>(end-end_phase1).count()<<"\n";
			
			cout << "TIME PHASE PER ITER = "  <<std::chrono::duration_cast<std::chrono::microseconds>(end-end_phase1).count()/iter<<"\n";
		}*/
	}
};

int main(int argc, char *argv[])
{
	srand (time(NULL));

	int total_points, total_values, K, max_iterations, has_name;

	cin >> total_points >> total_values >> K >> max_iterations >> has_name;

	vector<Point> points;
	string point_name;

	for(int i = 0; i < total_points; i++)
	{
		vector<double> values;

		for(int j = 0; j < total_values; j++)
		{
			double value;
			cin >> value;
			values.push_back(value);
		}

		if(has_name)
		{
			cin >> point_name;
			Point p(i, values, point_name);
			points.push_back(p);
		}
		else
		{
			Point p(i, values);
			points.push_back(p);
		}
	}

	KMeans kmeans(K, total_points, total_values, max_iterations);
	kmeans.run(points);

	return 0;
}
