
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/optflow/rlofflow.hpp>
// #include <opencv2/cudaoptflow.hpp>
// #include "opencv2/cudaarithm.hpp"
// #include "opencv2/imgproc.hpp"
// #include "opencv2/highgui.hpp"
// #include "opencv2/videoio.hpp"
// #include <opencv2/core/utility.hpp>
// #include <opencv2/video/tracking.hpp>

#include "optflowstats/predefstats.hpp"
#include "statcoll/collection.hpp"
#include "statcoll/collector.hpp"
#include "statcoll/collectionPrinter.hpp"

using namespace std;
using namespace ofs;
using namespace sc;
using namespace cv;
using namespace cv::cuda;

constexpr auto configFile = "config.json";

void drawOptFlowMap(const cv::Mat& flow, cv::Mat& cflowmap, int step, double, const Scalar& color)
{
    for(int y = 0; y < cflowmap.rows; y += step)
        for(int x = 0; x < cflowmap.cols; x += step)
        {
            const Point2f& fxy = flow.at<Point2f>(y, x);
            line(cflowmap, Point(x,y), Point(cvRound(x+/*300**/fxy.x), cvRound(y+/*300**/fxy.y)),
                 color);
            circle(cflowmap, Point(x,y), 1, color, -1);
        }
}

void showFlow(const char* name, const cv::Mat& flow, cv::Mat flowmap)
{
    drawOptFlowMap(flow,flowmap, 16, 1.5, Scalar(255, 255, 255));
    imshow(name, flowmap);
}

int main(int argc, char** argv)
{
    std::string videoFile = "";
    std::cout << "Starting flow analyzer" << std::endl;
    if(argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <video_file>" << std::endl;
        return -1;
    }
    else 
    {
        videoFile = argv[1];
    }

    cv::VideoCapture cap(videoFile);

    if( !cap.isOpened() ) 
    {
        std::cerr << "Cannot open the video file" << '\n';
        return -1;
    }
 
//  TODO: CUDA optical flow calculation
    cv::Mat frame0, frame1, frame;
    Ptr<optflow::RLOFOpticalFlowParameter> rlofParam = Ptr<optflow::RLOFOpticalFlowParameter>();
    Ptr<optflow::DenseRLOFOpticalFlow> RFLOflow=cv::optflow::DenseRLOFOpticalFlow::create( rlofParam,
		1.1f,
		Size(6, 6),
		optflow::InterpolationType::INTERP_GEO,
		128,
		0.05f,
		999.0f,
		15,
		100,
		true,
		500.0f,
		1.5f,
		false
	);

    try
    {
        Collector<Collection, CalcHoA, CalcMADiv> collector {configFile};

        int64_t flowTime = 0;
        int64_t histTime = 0;
        int iter = 5;
        for(int i = 0; i < iter; i++)
        {
            
            cap >> frame0;


            if(!frame1.empty())
            {
                cv::Mat flow(frame0.size(), CV_32FC2);
                cv::Mat frameMap(frame0);
                auto start = std::chrono::high_resolution_clock::now();
                RFLOflow->calc(frame0,frame1,flow);

                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
                if(i > 1) flowTime += duration.count()/(iter-1);
                
            #ifdef USE_GPU
                cv::cuda::GpuMat gpuFlow;
                gpuFlow.upload(flow);
                auto & flow_ref = gpuFlow;
            #else
                auto & flow_ref = flow;
            #endif

                start = std::chrono::high_resolution_clock::now();
                collector.run(flow_ref);
                stop = std::chrono::high_resolution_clock::now();
                auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                if(i > 1) histTime += duration2.count()/(iter-1);
            }
            swap(frame0, frame1);
        }

        std::cout << " Mean time for flow calculation: " << flowTime<< std::endl;
        std::cout << " Mean time for collector run: " << histTime << std::endl;
        
        auto collections = collector.getCollections();
        FilePrinter<JsonPrinter> printer;
        for(auto & collection : collections)
        {
            visit(printer, collection);
        }
    }
    catch(const CollectorException & e)
    {
        std::cerr << "EXCEPTION: " + string{e.what()} << '\n';
    }
    catch(const std::exception& e)
    {
        std::cerr << "EXCEPTION: " + string{e.what()} << '\n';
    }

    return 0;
}