#include "opencv2/core/core.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <vector>
#include <string>
#include <iostream>

using namespace cv;
using namespace std;

//
//
// opencv (cpu / totally unoptimized) adaption of the pseudocode found here:
//
// http://www2.ulg.ac.be/telecom/publi/publications/barnich/Barnich2011ViBe/index.html#toc-Section--2
//
// beware: it's patented!
//
//

struct Vibe
{
    int width;                     // width of the image
    int height;                    // height of the image
    int nbSamples;                 // number of samples per pixel
    int reqMatches;                // #_min matches
    int radius;                    // R^2
    int bogo_radius;               // adaptive radius when resizing /initializing the samples ( my addition ;] )
    int subsamplingFactor;         // amount of random subsampling

    vector<Mat_<uchar>> samples;   // the 'model'
    Mat_<uchar> segmentation;      // 0:bg , 255:fg

    RNG rng;

    Vibe(int w, int h, int nbSamples=10, int reqMatches=3, int radius=700, int subsamplingFactor=4)
        : width(w)
        , height(h)
        , nbSamples(nbSamples)
        , reqMatches(reqMatches)
        , radius(radius) // R^2
        , bogo_radius(200000)
        , subsamplingFactor(subsamplingFactor)
        , rng(getTickCount())
        , segmentation(height,width)
    {
        clear();
    }

    void clear()
    {
        samples.clear();
        for ( int i=0; i<nbSamples; i++ )
            samples.push_back( Mat_<uchar>(height,width,128) );
        bogo_radius= 200000;
    }

    void segment(const Mat & img, Mat & segmentationMap)
    {
        if ( nbSamples != samples.size() ) 
            clear();

        bogo_radius = bogo_radius > radius
                    ? bogo_radius *= 0.8
                    : radius;

        Mat_<uchar> image(img);
        for (int x=1; x<width-1; x++) // spare a 1 pixel border for the neighbour sampling
        {
            for (int y=1; y<height-1; y++)
            {
                uchar pixel = image(y,x);

                // comparison with the model
                int count = 0;
                for ( int i=0; (i<nbSamples)&&(count<reqMatches); i++ )
                {
                    int distance = pixel - samples[i](y,x);
                    count += (distance*distance < bogo_radius);
                }
                // pixel classification according to reqMatches
                if (count >= reqMatches) // the pixel belongs to the background
                {
                    // store 'bg' in the segmentation map
                    segmentation(y,x) = 0;
                    // gets a random number between 0 and subsamplingFactor-1
                    int randomNumber = rng.uniform(0, subsamplingFactor);
                    // update of the current pixel model
                    if (randomNumber == 0) // random subsampling
                    {
                        // other random values are ignored 
                        randomNumber = rng.uniform(0, nbSamples);
                        samples[randomNumber](y,x) = pixel;
                    }
                    // update of a neighboring pixel model
                    randomNumber = rng.uniform(0, subsamplingFactor);
                    if (randomNumber == 0) // random subsampling
                    {
                        // chooses a neighboring pixel randomly
                        const static int nb[8][2] = {-1,0, -1,1, 0,1, 1,1, 1,0, 1,-1, 0,-1, -1,-1};
                        int n = rng.uniform(0,8);
                        int neighborX = x + nb[n][1], neighborY = y + nb[n][0];
                        // chooses the value to be replaced randomly
                        randomNumber = rng.uniform(0, nbSamples);
                        samples[randomNumber](neighborY,neighborX) = pixel;
                    }    
                }
                else // the pixel belongs to the foreground 
                {    // store 'fg' in the segmentation map
                    segmentation(y,x) = 255;
                }
            }
        }
        segmentationMap = segmentation;
    }
};


int main( int argc, const char** argv )
{
    VideoCapture cap(0);
    if ( !cap.isOpened() )
        return -1; 

    int w = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int h = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT);

    Vibe vibe(w,h);

    namedWindow("controls");
    createTrackbar("samples","controls",&vibe.nbSamples,40);
    createTrackbar("radius", "controls",&vibe.radius,10000);
    createTrackbar("matches","controls",&vibe.reqMatches,10);
    createTrackbar("subsamp","controls",&vibe.subsamplingFactor,40);

    while(1)
    {
        Mat frame;
        if ( !cap.read(frame) ) continue;

        Mat gray;
        cvtColor(frame,gray, cv::COLOR_BGR2GRAY);
        
        Mat seg;
        vibe.segment(gray,seg);

        Mat segc;
        cvtColor(seg, segc, cv::COLOR_GRAY2BGR);
        addWeighted(frame,0.4, segc, 0.6, 0, frame );
        imshow("vibe",frame);
        //imshow("segm",seg);
        //imshow("d",vibe.samples[2]);

        int k = waitKey(10);
        if ( k == ' ' ) vibe.clear();
        if ( k == 27  ) break;
    }
    return 0;
}
