#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>
#include <wiringSerial.h>

using namespace cv;
using namespace std;

// Convert to string
#define SSTR(x) static_cast<std::ostringstream &>(           \
                    (std::ostringstream() << std::dec << x)) \
                    .str()

#define SERIAL_LOGGING
// #define TXT_FILE_LOGGING
// #define TXT_FPS_LOGGING

Point tgt_center;
Point tgt_center_normalized; // -1 <--> 1
float fps;

bool data_logging(const string data_type, const char *path, const char *operation)
{
    // // Creating File control object
    // FILE *Serial_file;

    // Serial_file = fopen(path, operation);
    // if (data_type == "Center_Point")
    // {
    //     fprintf(Serial_file, "$%d,% d*\n", tgt_center_normalized.x, tgt_center_normalized.y);
    // }
    // else if (data_type == "FPS")
    // {
    //     fprintf(Serial_file, "$%f*\n", fps);
    // }
    // fclose(Serial_file);

    // Creating File control object

    if (data_type == "Center_Point")
    {
        int handle = serialOpen("/dev/ttyAMA0", 9600);
        if (handle = -1)
        {
            cout << "Serial Error !!!" << endl;
            return false;
        }
        else
        {
            serialPrintf(handle, "$%d,% d*\n", tgt_center_normalized.x, tgt_center_normalized.y);
        }
    }
    else if (data_type == "FPS")
    {
        FILE *Serial_file;
        Serial_file = fopen(path, operation);
        fprintf(Serial_file, "$%f*\n", fps);
    }
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
    string trackerType = trackerTypes[4];
    cout << "Tracking Method : " + trackerType << endl;

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
    VideoCapture cap("../videos/3.mp4");

    int frame_width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    int frame_height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

    VideoWriter video("../videos/raspi_out.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, Size(frame_width, frame_height));

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
    Rect2d bbox(704, 429, 481, 183);

    // Uncomment the line below to select a different bounding box
    // bbox = selectROI(frame, false);
    putText(frame, SSTR((int)bbox.x) + "\t" + SSTR((int)bbox.y) + "\t" + SSTR((int)bbox.width) + "\t" + SSTR((int)bbox.height), Point(100, 20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);

    // Display bounding box.
    rectangle(frame, bbox, Scalar(255, 0, 0), 2, 1);

    // imshow("Tracking", frame);
    tracker->init(frame, bbox);

    while (cap.read(frame))
    {
        // Start timer
        double timer = (double)getTickCount();

        // Update the tracking result
        bool ok = tracker->update(frame, bbox);

        // Calculate Frames per second (FPS)
        fps = getTickFrequency() / ((double)getTickCount() - timer);

        if (ok)
        {
            // Tracking success : Draw the tracked object
            rectangle(frame, bbox, Scalar(255, 0, 0), 2, 1);

            tgt_center.x = bbox.x + 0.5 * bbox.width;
            tgt_center.y = bbox.y + 0.5 * bbox.height;
            circle(frame, tgt_center, 3, Scalar(0, 0, 255), 3, 5);

            tgt_center_normalized.x = 100 * (tgt_center.x - frame_width / 2) / (frame_width / 2);
            tgt_center_normalized.y = 100 * (tgt_center.y - frame_height / 2) / (frame_height / 2);

            putText(frame, "Target Center : " + SSTR(int(tgt_center.x)) + " , " + SSTR(int(tgt_center.y)), Point(100, 80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);
            cout << "FPS = " + SSTR(fps) << endl;
        }
        else
        {
            // Tracking failure detected.
            putText(frame, "Tracking failure detected", Point(100, 80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);
            cout << "FPS = Tracking Failed ( " << fps << " )" << endl;
        }
        // Display tracker type on frame
        putText(frame, trackerType + " Tracker", Point(100, 20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);

        // Display FPS on frame
        putText(frame, "FPS : " + SSTR(int(fps)), Point(100, 50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);

#ifdef SERIAL_LOGGING
        data_logging("Center_Point", "/dev/ttyAMA0", "w");
        // Serial_file = fopen("/dev/ttyACM0", "w");
        // fprintf(Serial_file, "$%d,% d*\n", tgt_center.x, tgt_center.y);
        // fclose(Serial_file);
#endif

#ifdef TXT_FILE_LOGGING
        data_logging("Center_Point", "../log/log.txt", "a");
        // Serial_file = fopen("../log/log.txt", "a");
        // fprintf(Serial_file, "$%d,% d*\n", tgt_center.x, tgt_center.y);
        // fclose(Serial_file);
#endif
#ifdef TXT_FPS_LOGGING
        data_logging("FPS", "../log/log.txt", "a");
        // Serial_file = fopen("../log/log.txt", "a");
        // fprintf(Serial_file, "$%d,% d*\n", tgt_center.x, tgt_center.y);
        // fclose(Serial_file);
#endif
        // Display frame.
        // imshow("Tracking", frame);

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

#ifdef TXT_FILE_LOGGING
    FILE *Serial_file;
    Serial_file = fopen("../log/log.txt", "a");
    fprintf(Serial_file, " ------------------------------- \n");
    fclose(Serial_file);
#endif
#ifdef TXT_FPS_LOGGING
    FILE *Serial_file;
    Serial_file = fopen("../log/log.txt", "a");
    fprintf(Serial_file, " ------------------------------- \n");
    fclose(Serial_file);
#endif

    destroyAllWindows();
}
