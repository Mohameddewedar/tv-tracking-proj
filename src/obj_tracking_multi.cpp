#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>

using namespace cv;
using namespace std;

// Convert to string
#define SSTR(x) static_cast<std::ostringstream &>(           \
                    (std::ostringstream() << std::dec << x)) \
                    .str()

// Point tgt_center;

// _BOOSTING
// _MIL
// _KCF
// _TLD
// _MEDIANFLOW
// _MOSSE
// _CSRT

int main(int argc, char **argv)
{
    // List of tracker types in OpenCV 3.4.1
    string trackerTypes[8] = {"BOOSTING", "MIL", "KCF", "TLD", "MEDIANFLOW", "GOTURN", "MOSSE", "CSRT"};
    // vector <string> trackerTypes(types, std::end(types));

    // Create a tracker
    string trackerType = trackerTypes[6];

    Ptr<Tracker> tracker_BOOSTING;
    Ptr<Tracker> tracker_MIL;
    Ptr<Tracker> tracker_KCF;
    Ptr<Tracker> tracker_TLD;
    Ptr<Tracker> tracker_MEDIANFLOW;
    Ptr<Tracker> tracker_MOSSE;
    Ptr<Tracker> tracker_CSRT;

#if (CV_MINOR_VERSION < 3)
    {
        tracker = Tracker::create(trackerType);
    }
#else
    {
        tracker_BOOSTING = TrackerBoosting::create();
        tracker_MIL = TrackerMIL::create();
        tracker_KCF = TrackerKCF::create();
        tracker_TLD = TrackerTLD::create();
        tracker_MEDIANFLOW = TrackerMedianFlow::create();
        tracker_MOSSE = TrackerMOSSE::create();
        tracker_CSRT = TrackerCSRT::create();
    }
#endif
    // Read video
    VideoCapture cap("../videos/2.mp4");

    int frame_width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    int frame_height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

    VideoWriter video("../videos/out.avi", CV_FOURCC('M', 'J', 'P', 'G'), 25, Size(frame_width, frame_height));

    // Exit if video is not opened
    if (!cap.isOpened())
    {
        cout << "Could not read video file" << endl;
        return 1;
    }

    // Read first frame
    Mat frame;
    bool ok = cap.read(frame);

    video.write(frame);

    // for (size_t i = 0; i < 30; i++)
    // {
    //     cap.read(frame);
    // }

    // Define initial bounding box
    Rect2d bbox(287, 23, 86, 320);
    Rect2d bbox_BOOSTING(287, 23, 86, 320);
    Rect2d bbox_MIL(287, 23, 86, 320);
    Rect2d bbox_KCF(287, 23, 86, 320);
    Rect2d bbox_TLD(287, 23, 86, 320);
    Rect2d bbox_MEDIANFLOW(287, 23, 86, 320);
    Rect2d bbox_MOSSE(287, 23, 86, 320);
    Rect2d bbox_CSRT(287, 23, 86, 320);

    // Uncomment the line below to select a different bounding box
    bbox = selectROI(frame, false);
    bbox_BOOSTING = bbox;
    bbox_MIL = bbox;
    bbox_KCF = bbox;
    bbox_TLD = bbox;
    bbox_MEDIANFLOW = bbox;
    bbox_MOSSE = bbox;
    bbox_CSRT = bbox;
    // Display bounding box.
    rectangle(frame, bbox, Scalar(255, 0, 0), 2, 1);

    imshow("Tracking", frame);

    tracker_BOOSTING->init(frame, bbox_BOOSTING);
    tracker_MIL->init(frame, bbox_MIL);
    tracker_KCF->init(frame, bbox_KCF);
    tracker_TLD->init(frame, bbox_TLD);
    tracker_MEDIANFLOW->init(frame, bbox_MEDIANFLOW);
    tracker_MOSSE->init(frame, bbox_MOSSE);
    tracker_CSRT->init(frame, bbox_CSRT);

    while (cap.read(frame))
    {

        double timer_BOOSTING = (double)getTickCount();
        bool ok_BOOSTING = tracker_BOOSTING->update(frame, bbox_BOOSTING);
        float fps_BOOSTING = getTickFrequency() / ((double)getTickCount() - timer_BOOSTING);

        if (ok_BOOSTING)
        {
            rectangle(frame, bbox_BOOSTING, Scalar(170, 170, 170), 2, 1);
            putText(frame, "FPS_BOOSTING : " + SSTR(int(fps_BOOSTING)), Point(50, 30), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(170, 170, 170), 2);
        }
        else
            putText(frame, "BOOSTING Tracking failure detected", Point(50, 30), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);

        double timer_MIL = (double)getTickCount();
        bool ok_MIL = tracker_MIL->update(frame, bbox_MIL);
        float fps_MIL = getTickFrequency() / ((double)getTickCount() - timer_MIL);

        if (ok_MIL)
        {
            rectangle(frame, bbox_MIL, Scalar(0, 255, 0), 2, 1);
            putText(frame, "FPS_MIL : " + SSTR(int(fps_MIL)), Point(50, 60), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 255, 0), 2);
        }
        else
            putText(frame, "MIL Tracking failure detected", Point(50, 60), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);

        double timer_KCF = (double)getTickCount();
        bool ok_KCF = tracker_KCF->update(frame, bbox_KCF);
        float fps_KCF = getTickFrequency() / ((double)getTickCount() - timer_KCF);

        if (ok_KCF)
        {
            rectangle(frame, bbox_KCF, Scalar(0, 170, 255), 2, 1);
            putText(frame, "FPS_KCF : " + SSTR(int(fps_KCF)), Point(50, 90), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 170, 255), 2);
        }
        else
            putText(frame, "KCF Tracking failure detected", Point(50, 90), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);

        double timer_TLD = (double)getTickCount();
        bool ok_TLD = tracker_TLD->update(frame, bbox_TLD);
        float fps_TLD = getTickFrequency() / ((double)getTickCount() - timer_TLD);

        if (ok_TLD)
        {
            rectangle(frame, bbox_TLD, Scalar(255, 255, 0), 2, 1);
            putText(frame, "FPS_TLD : " + SSTR(int(fps_TLD)), Point(50, 120), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(255, 255, 0), 2);
        }
        else
            putText(frame, "TLD Tracking failure detected", Point(50, 120), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);

        double timer_MEDIANFLOW = (double)getTickCount();
        bool ok_MEDIANFLOW = tracker_MEDIANFLOW->update(frame, bbox_MEDIANFLOW);
        float fps_MEDIANFLOW = getTickFrequency() / ((double)getTickCount() - timer_MEDIANFLOW);

        if (ok_MEDIANFLOW)
        {
            rectangle(frame, bbox_MEDIANFLOW, Scalar(0, 255, 255), 2, 1);
            putText(frame, "FPS_MEDIANFLOW : " + SSTR(int(fps_MEDIANFLOW)), Point(50, 150), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 255, 255), 2);
        }
        else
            putText(frame, "MEDIANFLOW Tracking failure detected", Point(50, 150), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);

        double timer_MOSSE = (double)getTickCount();
        bool ok_MOSSE = tracker_MOSSE->update(frame, bbox_MOSSE);
        float fps_MOSSE = getTickFrequency() / ((double)getTickCount() - timer_MOSSE);

        if (ok_MOSSE)
        {
            rectangle(frame, bbox_MOSSE, Scalar(150, 0, 200), 2, 1);
            putText(frame, "FPS_MOSSE : " + SSTR(int(fps_MOSSE)), Point(50, 180), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(150, 0, 200), 2);
        }
        else
            putText(frame, "MOSSE Tracking failure detected", Point(50, 180), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);

        double timer_CSRT = (double)getTickCount();
        bool ok_CSRT = tracker_CSRT->update(frame, bbox_CSRT);
        float fps_CSRT = getTickFrequency() / ((double)getTickCount() - timer_CSRT);

        if (ok_CSRT)
        {
            rectangle(frame, bbox_CSRT, Scalar(255, 255, 255), 2, 1);
            putText(frame, "FPS_CSRT : " + SSTR(int(fps_CSRT)), Point(50, 210), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(255, 255, 255), 2);
        }
        else
            putText(frame, "CSRT Tracking failure detected", Point(50, 210), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);

        // Display frame.
        imshow("Tracking", frame);

        video.write(frame);

        // Exit if ESC pressed.
        int k = waitKey(1);
        if (k == 27)
        {
            break;
        }
    }
    cap.release();
    video.release();

    destroyAllWindows();
}