#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>

#include <iostream>
#include <stdio.h>
// #include <time.h>   // In case delay is needed

using namespace cv;
using namespace std;

// Convert to string
#define SSTR(x) static_cast<std::ostringstream &>(           \
                    (std::ostringstream() << std::dec << x)) \
                    .str()

void dram_crossbow(Mat &frame, int frame_width, int frame_height)
{
    Scalar color(255, 10, 10);

    line(frame, Point(frame_width / 2, 0), Point(frame_width / 2, frame_height), color, 2, 16);
    line(frame, Point(frame_width, frame_height / 2), Point(0, frame_height / 2), color, 2, 16);

    circle(frame, Point(frame_width / 2, frame_height / 2), 50, color);
}

Point tgt_center;
Point tgt_center_normalized;

Point last_tgt_center;
double delta_x = 0, delta_y = 0;

// #define ENABLE_RETRACKING

// #define SERIAL_LOGGING
// #define TXT_FILE_LOGGING

bool serial_logging_started = false;
bool txt_logging_started = false;

bool matching_fn(Mat frame, Mat templ, int match_method, float match_quality = 0.9, Mat &result = *(new Mat), Point &matchloc = *(new Point))
// bool matching_fn(Mat frame, Mat templ, int match_method )
{
    try
    {

        double minVal;
        double maxVal;
        Point minLoc;
        Point maxLoc;
        Point matchLoc;

        if (frame.cols < templ.cols && frame.rows < templ.rows)
        {
            int result_cols = templ.cols - frame.cols + 1;
            int result_rows = templ.rows - frame.rows + 1;

            result.create(result_rows, result_cols, CV_32FC1);

            matchTemplate(templ, frame, result, match_method);
        }
        else
        {
            if (frame.cols < templ.cols)
            {
                templ = templ.colRange((templ.cols - frame.cols) / 2, (templ.cols + frame.cols) / 2);
            }

            if (frame.rows < templ.rows)
            {
                templ = templ.colRange((templ.rows - frame.rows) / 2, (templ.rows + frame.rows) / 2);
            }

            int result_cols = frame.cols - templ.cols + 1;
            int result_rows = frame.rows - templ.rows + 1;

            result.create(result_rows, result_cols, CV_32FC1);

            matchTemplate(frame, templ, result, match_method);
        }
        minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

        if ((match_method == CV_TM_SQDIFF_NORMED && minVal < (1 - match_quality)) || (match_method != CV_TM_SQDIFF_NORMED && maxVal > match_quality))
        {
            //Accept the match
            if (match_method == TM_SQDIFF || match_method == TM_SQDIFF_NORMED)
            {
                matchLoc = minLoc;
            }
            else
            {
                matchLoc = maxLoc;
            }
            return true;
        }
        else
        {
            return false;
        }
    }
    catch (cv::Exception &e)
    {
        // destroyWindow("Target Exctracted");
        // bool track_exctracted_error = true;
        const char *err_msg = e.what();

        cout << "exception caught: " << err_msg << std::endl;
        return false;
    }
}

FILE *Serial_file;
bool data_logging(const char *path, const char *operation, string status = "update")
{
    // Creating File control object

    if (!serial_logging_started)
    {
        Serial_file = fopen(path, operation);
        serial_logging_started = true;
    }

    if (!txt_logging_started)
    {
        Serial_file = fopen(path, operation);
        txt_logging_started = true;
    }
    fprintf(Serial_file, "$%d,%d*\n", tgt_center_normalized.x, tgt_center_normalized.y);

    cout << "$" << tgt_center_normalized.x << "," << tgt_center_normalized.y << "*" << endl;

    if (status == "ending")
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
    string trackerType = trackerTypes[0];

    // Choose a Match Template Method
    int match_method = 1; // 1 :TM_SQDIFF_NORMED ,3 :TM_CCORR_NORMED ,5 :TM_CCOEFF_NORMED

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
    VideoCapture cap("../videos/2.mp4");
    // VideoCapture cap(0);
    // VideoCapture cap("udp://127.0.0.1:9000", CAP_FFMPEG);
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

    bool ok = cap.read(frame);

    flip(frame, frame, 1);

    video.write(frame);

    // for (size_t i = 0; i < 30; i++)
    // {
    //     cap.read(frame);
    // }

    // Define initial bounding box
    Rect2d bbox(287, 23, 86, 320);

    last_tgt_center.x = bbox.x + 0.5 * bbox.width;
    last_tgt_center.y = bbox.y + 0.5 * bbox.height;

    // Uncomment the line below to select a different bounding box
    // Please remember to select small portion of the tgt.
    // putText(frame, "Please Select Small Dstinctive Part Of The target.", Point(100, 56), FONT_HERSHEY_SIMPLEX, 1.25, Scalar(50, 170, 50), 3);
    // bbox = selectROI(frame, false);
    // Display bounding box.
    rectangle(frame, bbox, Scalar(255, 0, 0), 2, 1);

    dram_crossbow(frame, frame_width, frame_height);
    imshow("Tracking", frame);

    setMouseCallback("Tracking", onMouse, 0);
    // tracker->init(frame, bbox);

    Mat backuped_frames[16];

    // croppedRef.copyTo(backuped_frames[((int)cap.get(CV_CAP_PROP_POS_FRAMES) - 1) % 15] );
    int num_good_backup_frames = 1;
    int seq_good_imgs = 0;
    //double last_timer = (double)getTickCount();

    // Mat fframe;
    while (cap.read(frame))
    {
        // flip(fframe, frame, 0);

        // Start timer
        double timer = (double)getTickCount();
        if (!valid_selection_available && select_no != 0)
        {
            // Update the tracking result
            bool ok = tracker->update(frame, bbox);

            // Calculate Frames per second (FPS)
            float fps = getTickFrequency() / ((double)getTickCount() - timer);

            // cout << "bbox output string" << bbox << endl;

            if (ok)
            {
                //Take a Backuped image
                bool track_exctracted_error = false;
                // cout << bbox << endl;
                Rect temp_bbox = bbox;
                if (bbox.x < 0)
                {
                    bbox.width -= bbox.x;
                    bbox.x = 0;
                }
                if (bbox.y < 0)
                {
                    bbox.height -= bbox.y;
                    bbox.y = 0;
                }
                if (bbox.x + bbox.width > frame_width)
                {
                    bbox.width -= bbox.x - frame_width;
                    bbox.x = frame_width - bbox.width;
                }
                if (bbox.y + bbox.height > frame_height)
                {
                    bbox.height -= bbox.width + frame_height;
                    bbox.y = frame_height - bbox.height;
                }

                Mat croppedRef(frame, bbox);

                bool matching_OK = matching_fn(croppedRef, backuped_frames[15], match_method, 0.85);
                bbox = temp_bbox;

                if (matching_OK)
                {
                    croppedRef.copyTo(backuped_frames[seq_good_imgs]);
                    num_good_backup_frames++;
                    imshow("Good Sample", backuped_frames[seq_good_imgs]);
                    // cout << "Good Sample" << endl;

                    if (seq_good_imgs >= 14)
                    {
                        seq_good_imgs = 0;
                    }
                    else
                    {
                        seq_good_imgs++;
                    }
                }
                else
                {
                    // cout << "not a good Sample" << endl;
                }

                // if (!track_exctracted_error)
                // {
                //     imshow("Target Exctracted", backuped_frames[seq_good_imgs]);
                // }
                // Tracking success : Draw the tracked object
                if (!selecting)

                    rectangle(frame, bbox, Scalar(255, 0, 0), 2, 1);

                tgt_center.x = bbox.x + 0.5 * bbox.width;
                tgt_center.y = bbox.y + 0.5 * bbox.height;

                circle(frame, tgt_center, 3, Scalar(0, 0, 255), 3, 5);

                tgt_center_normalized.x = 100 * (tgt_center.x - frame_width / 2) / (frame_width / 2);
                tgt_center_normalized.y = 100 * (tgt_center.y - frame_height / 2) / (frame_height / 2);

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
                dram_crossbow(frame, frame_width, frame_height);
                imshow("Tracking", frame);

#ifdef ENABLE_RETRACKING

                // Later Make Functions ...
                // Begin Retrack Process
                Point matchLoc;
                Mat result;
                // The Original frame first of all at index (-1)

                for (int i = -1; i < 15; i++)
                {
                    Mat result, templ;
                    if (i == -1)
                        templ = backuped_frames[15];
                    else
                        templ = backuped_frames[seq_good_imgs];
                    cout << i << endl;
                    imshow("The used Template", templ);

                    bool matching_OK = matching_fn(frame, templ, match_method, 0.95, result, matchLoc);

                    if (matching_OK)
                    {

                        bbox = Rect2d(matchLoc, Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows));

                        Mat croppedRef(frame, bbox);

                        imshow("Retracked Target", croppedRef);

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

                        break;
                    }
                    else
                    {
                        putText(frame, "Retracking Failed ", Point(1000, 50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(255, 0, 0), 2);
                        cout << "i didn't Accepted the match" << endl;
                    }
                }

#endif
            }
            // Display tracker type on frame
            putText(frame, trackerType + " Tracker", Point(100, 20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);

            // Display FPS on frame
            putText(frame, "FPS : " + SSTR(int(fps)), Point(100, 50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);
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

            Mat croppedRef(frame, bbox);
            croppedRef.copyTo(backuped_frames[15]);

            select_no++;
            valid_selection_available = false;
        }
        if (selecting)
            rectangle(frame, selectBox, Scalar(255, 0, 0), 2, 1);

#ifdef ENABLE_RETRACKING
        putText(frame, "Retracking Enabled ", Point(1000, 20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(255, 0, 0), 2);
#endif

        // Display frame.
        dram_crossbow(frame, frame_width, frame_height);
        imshow("Tracking", frame);
        last_tgt_center = tgt_center;

        video.write(frame);

        // Exit if ESC pressed.
        int k = waitKey(1);
        if (k == 27)
        {
            break;
        }
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