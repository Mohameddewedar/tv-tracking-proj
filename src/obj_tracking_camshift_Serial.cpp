#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
// #include "opencv2/highgui.hpp"

#include <iostream>
#include <stdio.h>
#include <ctype.h>

// #include <time.h>   // In case delay is needed

using namespace cv;
using namespace std;

// Convert to string
#define SSTR(x) static_cast<std::ostringstream &>(           \
                    (std::ostringstream() << std::dec << x)) \
                    .str()

#define SERIAL_LOGGING
// #define TXT_FILE_LOGGING

Point tgt_center;
Point last_tgt_center;
double delta_x = 0, delta_y = 0;

bool data_logging(const char *path, const char *operation)
{
    // Creating File control object
    FILE *Serial_file;

    Serial_file = fopen(path, operation);
    fprintf(Serial_file, "$%d,% d*\n", tgt_center.x, tgt_center.y);
    fclose(Serial_file);
}

bool selectObject = false;
bool valid_selection_available = false;
Point start_select_point;
Point final_select_point;
Rect2d selectBox;
Rect2d last_valid_selectBox;
int select_no = 0;
bool selecting = false;

static void onMouse(int event, int x, int y, int, void *)
{

    if (selecting)
    {
        final_select_point.x = x;
        final_select_point.y = y;

        selectBox = Rect2d(start_select_point, final_select_point);
    }

    switch (event)
    {

    case EVENT_LBUTTONDOWN:
        start_select_point.x = x;
        start_select_point.y = y;

        valid_selection_available = false;
        selecting = true;

        break;
    case EVENT_LBUTTONUP:
        final_select_point.x = x;
        final_select_point.y = y;
        selecting = false;
        if (selectBox.width > 1 && selectBox.height > 1)
        {                                     // if valid
            valid_selection_available = true; // Set up CAMShift properties in main() loop
            last_valid_selectBox = selectBox;
        }
        else
        {
            selectBox = last_valid_selectBox;
        }

        break;
    }
}

int main(int argc, char **argv)
{

    // List of tracker types in OpenCV 3.4.1
    string trackerTypes[8] = {"BOOSTING", "MIL", "KCF", "TLD", "MEDIANFLOW", "GOTURN", "MOSSE", "CSRT"};
    // vector <string> trackerTypes(types, std::end(types));

    // Create a tracker
    string trackerType = trackerTypes[2];

    Ptr<Tracker> tracker;

#if (CV_MINOR_VERSION < 3)
    {
        tracker = Tracker::create(trackerType);
    }
#else
    {
        if (trackerType == "BOOSTING")
            tracker = TrackerBoosting::create();
        if (trackerType == "MIL")
            tracker = TrackerMIL::create();
        if (trackerType == "KCF")
            tracker = TrackerKCF::create();
        if (trackerType == "TLD")
            tracker = TrackerTLD::create();
        if (trackerType == "MEDIANFLOW")
            tracker = TrackerMedianFlow::create();
        if (trackerType == "GOTURN")
            tracker = TrackerGOTURN::create();
        if (trackerType == "MOSSE")
            tracker = TrackerMOSSE::create();
        if (trackerType == "CSRT")
            tracker = TrackerCSRT::create();
    }
#endif
    // Read video
    // VideoCapture cap("../videos/2.mp4",CAP_FFMPEG);
    // VideoCapture cap(0);
    VideoCapture cap("udp://127.0.0.1:9000", CAP_FFMPEG);
    // VideoCapture cap("udpsrc port=9000 caps='application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264' ! rtph264depay ! avdec_h264 ! videoconvert ! autovideosink sync=f",CAP_GSTREAMER);
    // VideoCapture cap("udpsrc port=9000 caps=\"application/x-rtp\" ! rtph264depay ! avdec_h264 ! videoconvert ! xvimagesink",CAP_GSTREAMER);
    
    cap.set(CV_CAP_PROP_BUFFERSIZE, 3);
    
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
    cap.read(frame);

    video.write(frame);

    // Define initial bounding box
    Rect2d bbox(287, 23, 86, 320);

    last_tgt_center.x = bbox.x + 0.5 * bbox.width;
    last_tgt_center.y = bbox.y + 0.5 * bbox.height;

    // Uncomment the line below to select a different bounding box
    // bbox = selectROI(frame, false);
    // Display bounding box.
    // rectangle(frame, bbox, Scalar(255, 0, 0), 2, 1);

    imshow("Tracking", frame);
    setMouseCallback("Tracking", onMouse, 0);
    // tracker->init(frame, bbox);

    //double last_timer = (double)getTickCount();

    while (cap.read(frame))
    {
        // Start timer
        double timer = (double)getTickCount();
        if (!valid_selection_available && select_no != 0)
        {

            // Update the tracking result
            bool ok = tracker->update(frame, bbox);

            // Calculate Frames per second (FPS)
            float fps = getTickFrequency() / ((double)getTickCount() - timer);

            if (ok)
            {
                // Tracking success : Draw the tracked object
                if (!selecting)
                    rectangle(frame, bbox, Scalar(255, 0, 0), 2, 1);

                tgt_center.x = bbox.x + 0.5 * bbox.width;
                tgt_center.y = bbox.y + 0.5 * bbox.height;

                circle(frame, tgt_center, 3, Scalar(0, 0, 255), 3, 5);

                tgt_center.x -= frame_width / 2;
                tgt_center.y -= frame_height / 2;

                delta_x = 1000 * tgt_center.x * (tgt_center.x - last_tgt_center.x) / frame_width;
                delta_y = 1000 * tgt_center.y * (tgt_center.y - last_tgt_center.y) / frame_height;

                putText(frame, "Target Center : " + SSTR(int(tgt_center.x)) + " , " + SSTR(int(tgt_center.y)), Point(100, 80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);
                putText(frame, "Delta X : " + SSTR(int(delta_x)) + " , Delta Y : " + SSTR(int(delta_y)), Point(100, 110), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);

                // Sending the Data to Arduino (Serial)

#ifdef SERIAL_LOGGING
                data_logging("/dev/ttyACM0", "w");
                // Serial_file = fopen("/dev/ttyACM0", "w");
                // fprintf(Serial_file, "$%d,% d*\n", tgt_center.x, tgt_center.y);
                // fclose(Serial_file);
#endif

#ifdef TXT_FILE_LOGGING
                data_logging("../log/log.txt", "a");
                // Serial_file = fopen("../log/log.txt", "a");
                // fprintf(Serial_file, "$%d,% d*\n", tgt_center.x, tgt_center.y);
                // fclose(Serial_file);
#endif
            }
            else
            {
                // Tracking failure detected.
                putText(frame, "Tracking failure detected", Point(100, 80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);
            }
            // Display tracker type on frame
            putText(frame, trackerType + " Tracker", Point(100, 20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);

            // Display FPS on frame
            putText(frame, "FPS : " + SSTR(int(fps)), Point(100, 50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);

            // Display frame.

            last_tgt_center = tgt_center;
        }
        else if (valid_selection_available)
        {
            bbox = selectBox;

            if (trackerType == "BOOSTING")
                tracker = TrackerBoosting::create();
            if (trackerType == "MIL")
                tracker = TrackerMIL::create();
            if (trackerType == "KCF")
                tracker = TrackerKCF::create();
            if (trackerType == "TLD")
                tracker = TrackerTLD::create();
            if (trackerType == "MEDIANFLOW")
                tracker = TrackerMedianFlow::create();
            if (trackerType == "GOTURN")
                tracker = TrackerGOTURN::create();
            if (trackerType == "MOSSE")
                tracker = TrackerMOSSE::create();
            if (trackerType == "CSRT")
                tracker = TrackerCSRT::create();

            tracker->init(frame, bbox);

            select_no++;
            valid_selection_available = false;
        }

        if (selecting)
            rectangle(frame, selectBox, Scalar(255, 0, 0), 2, 1);

        putText(frame, "Selection Number : " + SSTR(int(select_no)), Point(100, 150), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);
        imshow("Tracking", frame);
        video.write(frame);

        // Exit if ESC pressed.
        int k = waitKey(1);
        if (k == 27)
        {
            break;
        }

        // if (((double)getTickCount() - timer) / getTickFrequency() < 0.1)  // delay
        //     waitKey(0.1 - (((double)getTickCount() - timer) / getTickFrequency()));
    }

#ifdef TXT_FILE_LOGGING
    FILE *Serial_file;
    Serial_file = fopen("../log/log.txt", "a");
    fprintf(Serial_file, " ------------------------------- \n");
    fclose(Serial_file);
#endif

    cap.release();
    video.release();

    destroyAllWindows();
}