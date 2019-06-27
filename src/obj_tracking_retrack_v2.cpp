// Very Acceptable Preformance
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

Point tgt_center;
Point last_tgt_center;
double delta_x = 0, delta_y = 0;

#define ENABLE_RETRACKING

// #define SERIAL_LOGGING
#define TXT_FILE_LOGGING

bool data_logging(const char *path, const char *operation)
{
    // Creating File control object
    FILE *Serial_file;

    Serial_file = fopen(path, operation);
    fprintf(Serial_file, "$%d,% d*\n", tgt_center.x, tgt_center.y);
    fclose(Serial_file);
}

int main(int argc, char **argv)
{

    // List of tracker types in OpenCV 3.4.1
    string trackerTypes[8] = {"BOOSTING", "MIL", "KCF", "TLD", "MEDIANFLOW", "GOTURN", "MOSSE", "CSRT"};
    // vector <string> trackerTypes(types, std::end(types));

    // Create a tracker
    string trackerType = trackerTypes[2];

    // Choose a Match Template Method
    int match_method = 3; // 1 :TM_SQDIFF_NORMED ,3 :TM_CCORR_NORMED ,5 :TM_CCOEFF_NORMED

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

    last_tgt_center.x = bbox.x + 0.5 * bbox.width;
    last_tgt_center.y = bbox.y + 0.5 * bbox.height;

    // Uncomment the line below to select a different bounding box
    bbox = selectROI(frame, false);
    // Display bounding box.
    rectangle(frame, bbox, Scalar(255, 0, 0), 2, 1);
    
    imshow("Tracking", frame);
    waitKey();
    
    tracker->init(frame, bbox);

    Mat croppedRef(frame, bbox);
    Mat backuped_frames[16];
    croppedRef.copyTo(backuped_frames[15]);
    // croppedRef.copyTo(backuped_frames[((int)cap.get(CV_CAP_PROP_POS_FRAMES) - 1) % 15] );
    int num_good_backup_frames = 1;
    int seq_good_imgs = 0;
    //double last_timer = (double)getTickCount();

    while (cap.read(frame))
    {

        // Start timer
        double timer = (double)getTickCount();

        // Update the tracking result
        bool ok = tracker->update(frame, bbox);

        // Calculate Frames per second (FPS)
        float fps = getTickFrequency() / ((double)getTickCount() - timer);

        if (ok)
        {
            //Take a Backuped image
            bool track_exctracted_error = false;
            try
            {
                Mat croppedRef(frame, bbox), result;
                matchTemplate(croppedRef, backuped_frames[15], result, match_method);
                double minVal, maxVal;
                minMaxLoc(result, &minVal, &maxVal);
                double match_quality = 0.9;
                if ((match_method == CV_TM_SQDIFF_NORMED && minVal < (1 - match_quality)) || (match_method != CV_TM_SQDIFF_NORMED && maxVal > match_quality))
                {
                    croppedRef.copyTo(backuped_frames[seq_good_imgs]);
                    num_good_backup_frames++;
                    imshow("Good Sample", backuped_frames[seq_good_imgs]);

                    if (seq_good_imgs >= 14)
                    {
                        seq_good_imgs = 0;
                    }
                    else
                    {
                        seq_good_imgs++;
                    }
                }
            }
            catch (cv::Exception &e)
            {
                //destroyWindow("Target Exctracted");
                bool track_exctracted_error = true;
                const char *err_msg = e.what();
                cout << "exception caught: " << err_msg << std::endl;
            }
            // if (!track_exctracted_error)
            // {
            //     imshow("Target Exctracted", backuped_frames[seq_good_imgs]);
            // }
            // Tracking success : Draw the tracked object
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
            imshow("Tracking", frame);

#ifdef ENABLE_RETRACKING
            {
                // Later Make Functions ...
                // Begin Retrack Process
                double minVal;
                double maxVal;
                Point minLoc;
                Point maxLoc;
                Point matchLoc;

                // The Original frame first of all at index (-1)

                for (int i = -1; i < 15; i++)
                {
                    Mat result, templ;
                    if (i == -1)
                        templ = backuped_frames[15];
                    else
                        templ = backuped_frames[seq_good_imgs];

                    imshow("The used Template", templ);

                    int result_cols = frame.cols - templ.cols + 1;
                    int result_rows = frame.rows - templ.rows + 1;

                    result.create(result_rows, result_cols, CV_32FC1);

                    matchTemplate(frame, templ, result, match_method);

                    minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

                    float match_quality = 0.95;

                    cout << "i am in the loop " << endl;

                    if ((match_method == CV_TM_SQDIFF_NORMED && minVal < (1 - match_quality)) || (match_method != CV_TM_SQDIFF_NORMED && maxVal > match_quality))
                    {
                        //Accept the match
                        cout << "i Accepted the match" << endl;
                        if (match_method == TM_SQDIFF || match_method == TM_SQDIFF_NORMED)
                        {
                            matchLoc = minLoc;
                            cout << "Minimum Value is : " << minVal << endl;
                        }
                        else
                        {
                            matchLoc = maxLoc;
                            cout << "Maximum Value is : " << maxVal << endl;
                        }
                        bbox = Rect2d(matchLoc, Point(matchLoc.x + templ.cols, matchLoc.y + backuped_frames[seq_good_imgs].rows));

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
            }
#endif
        }
        // Display tracker type on frame
        putText(frame, trackerType + " Tracker", Point(100, 20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);

        // Display FPS on frame
        putText(frame, "FPS : " + SSTR(int(fps)), Point(100, 50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);

#ifdef ENABLE_RETRACKING
        putText(frame, "Retracking Enabled ", Point(1000, 20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(255, 0, 0), 2);
#endif

        // Display frame.
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