#include <stdio.h>
#include <iostream>
#include "tinydir.h"
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/features2d.hpp"

using namespace std;
using namespace cv;

void readme();
int processArgs(int argc, char** argv);
int capture(string output_file);
int compare(Mat img_1, Mat img_2);
int find(string search_dir);
int camera();

bool cap = false;
bool comp = false;
bool fi = false;

Mat mat;
string input_file1, input_file2;
string output_file, search_dir; 

int main( int argc, char** argv ) {

  if(processArgs(argc, argv) == -1) {
    return -1;
  }

  if(cap) {
    if(capture(output_file) == -1){
      return -1;
    }
  } 

  if(comp) {
    if(!input_file1.empty() && !input_file2.empty()){

      Mat img_1 = imread( input_file1, IMREAD_GRAYSCALE );
      Mat img_2 = imread( input_file2, IMREAD_GRAYSCALE );

      if( !img_1.data || !img_2.data ) { 
        std::cout<< "Error reading images." << std::endl; return -1; 
      }

      std::cout << compare(img_1, img_2) << std::endl; 
    }
  } 

  if(fi) {
    if(search_dir.empty()){
      std::cout << "Missing search directory" << std::endl;
      return -1;
    }

    if(find(search_dir) == -1) {
      return -1;
    }
  }

  if(!cap && !comp && !fi) {
    readme(); return -1;
  } 

  return 0;
}

int find(string search_dir) { 
  
  if(camera() == -1){
    return -1;
  }

  tinydir_dir dir;
  tinydir_open(&dir, search_dir.c_str());

  int best_matches = 0;
  string best_path;

  while (dir.has_next) {
    tinydir_file file;
    tinydir_readfile(&dir, &file);

    if (!file.is_dir){
      Mat img_1 = imread(file.path, IMREAD_GRAYSCALE );
      
      int matches = compare(mat, img_1);
      std::cout << file.path << " : " << matches << std::endl;

      if(matches > best_matches) {
        best_matches = matches;
        best_path = string(file.path); 
      }
    }
    
    tinydir_next(&dir);
  }

  std::cout << best_path << std::endl;

  return 0;
}


int compare(Mat img_1, Mat img_2) {
  //-- Step 1: Detect the keypoints using SURF Detector
  int minHessian = 400;

  SurfFeatureDetector detector( minHessian );

  std::vector<KeyPoint> keypoints_1, keypoints_2;

  detector.detect( img_1, keypoints_1 );
  detector.detect( img_2, keypoints_2 );

  //-- Step 2: Calculate descriptors (feature vectors)
  SurfDescriptorExtractor extractor;

  Mat descriptors_1, descriptors_2;

  extractor.compute( img_1, keypoints_1, descriptors_1 );
  extractor.compute( img_2, keypoints_2, descriptors_2 );

  //-- Step 3: Matching descriptor vectors using FLANN matcher
  FlannBasedMatcher matcher;
  std::vector< DMatch > matches;
  matcher.match( descriptors_1, descriptors_2, matches );

  double max_dist = 0; double min_dist = 100;

  //-- Quick calculation of max and min distances between keypoints
  for( int i = 0; i < descriptors_1.rows; i++ ) { 
    double dist = matches[i].distance;
    if( dist < min_dist ) min_dist = dist;
    if( dist > max_dist ) max_dist = dist;
  }

  //-- Draw only "good" matches (i.e. whose distance is less than 2*min_dist,
  //-- or a small arbitary value ( 0.02 ) in the event that min_dist is very
  //-- small)
  //-- PS.- radiusMatch can also be used here.
  std::vector< DMatch > good_matches;

  for( int i = 0; i < descriptors_1.rows; i++ ) { 
    if( matches[i].distance <= max(5 * min_dist, 0.02) ) { 
      good_matches.push_back( matches[i]); 
    }
  }

  Mat img_matches;
  drawMatches( img_1, keypoints_1, img_2, keypoints_2,
               good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
               vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

  //-- Show detected matches
  imshow( "Good Matches", img_matches );

  waitKey(0);

  return good_matches.size();
}

int capture(string output_file) {
  if(output_file.empty()){
    std::cout << "Missing output file" << std::endl;
    return -1;
  }

  if(camera() == -1){
    return -1;
  }

  try {
    imwrite(output_file, mat);
  } catch (std::exception& ex) {
    fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
    return -1;
  }

  fprintf(stdout, "Saved image file.\n");
  return 0;
}

int camera(){
  VideoCapture cap(0); // open the default camera
    
  if(!cap.isOpened()) {  // check if we succeeded
    std::cout << " Unable to access camera." << std::endl; 
    return -1;
  }
  
  cap >> mat;
  return 0;
}

int processArgs(int argc, char** argv) {
  for(int i = 0; i < argc; i++) {
    String arg(argv[i]);
    if(arg.compare("-i") == 0) {
      if(i + 2 > argc){
        readme(); return -1;
      } else {
        comp = true;
        input_file1 = argv[++i];
        input_file2 = argv[++i];
      }
    }

    if(arg.compare("-c") == 0) {
      cap = true;
    }

    if(arg.compare("-o") == 0) {
      if(i + 1 > argc) {
        readme(); return -1;
      } else {
        output_file = argv[++i];
      }
    } 

    if(arg.compare("-f") == 0) {
      fi = true;
    } 

    if(arg.compare("-d") == 0) {
      if(i + 1 > argc) {
        readme(); return -1;
      } else {
        search_dir = argv[++i];
      }
    } 
  }
  return 0;
}

void readme() { 
  std::cout << " Usage: ./CMatcher [-i input_file input_file] [-c -o output_file] [-f -d search_dir]" << std::endl; 
}