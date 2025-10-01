//
//  main.cpp
//  Boarding-System
//
//  Created by Brian Soh on 2025-09-15.
//

#include "main.h"

namespace py = pybind11;
using namespace std;
using namespace cv;
using namespace pybind11;
using namespace filesystem;

// Helper function to convert Mat datatype to numPy array
py::array mat_to_numpy(const Mat& mat) {
    Mat c;
    // Ensures no gaps so copy to numPy is continuous
    if (mat.isContinuous()) {
        c = mat;
    }
    else {
        c = mat.clone();
    }
    
    // Move data into a heap to control lifetime
    auto m = new Mat(std::move(c));
    
    vector<ssize_t> shape, strides;
    shape   = { m->rows, m->cols, m->channels() };
    strides = { (ssize_t)m->step, (ssize_t)m->elemSize(),(ssize_t)1};

    // Attach capsule to control lifetime
    py::capsule free_when_done(m, [](void* p){ delete reinterpret_cast<Mat*>(p);});
    
    return py::array(
        py::buffer_info(
            m->data, sizeof(uint8_t),
            py::format_descriptor<uint8_t>::format(),
            (int)shape.size(), shape, strides
        ),
        free_when_done
    );
}

int main() {
    
    std::unique_ptr<pqxx::connection> c;
    
    try {
        std::string conn =
        "host=xxx "
        "port=5432 "
        "dbname=boarding_system "
        "user=postgres "
        "password=xxx "
        "sslmode=verify-full "
        "sslrootcert=Resources/us-west-2-bundle.pem";
                
        c = std::make_unique<pqxx::connection>(conn);
        
        if (!c->is_open()) throw std::runtime_error("DB not open");
        cout << "DB Connected" << endl;
        
    } catch (const std::exception& e) {
        cerr << "Error: " << e.what() << endl;
        return -1;
    }
    
    // Start Python
    scoped_interpreter guard{};
    auto sys = module_::import("sys");
    sys.attr("path").attr("append")("/Users/briansoh/Desktop/Coding Projects/Air-Zenith/venv/lib/python3.13/site-packages");
        
    auto fr = module_::import("face_recognition");
    auto cv2 = module_::import("cv2");
    auto np = module_::import("numpy");

    vector<Mat> faces;
    vector<string> ids;
    
    string folder = "Images";
    
    py::list encodings;
    
    for (const auto& entry : directory_iterator(folder)) {
        if (entry.path().extension().string() != ".png") continue;
        object image = cv2.attr("imread")(entry.path().string());
        object rgb = cv2.attr("cvtColor")(image, cv2.attr("COLOR_BGR2RGB"));
        py::list encoding = fr.attr("face_encodings")(rgb);
        py::list firstenc = encoding[0];
        encodings.append(firstenc);
        string id = entry.path().stem().string();
        ids.push_back(id);
    }
        
    if (ids.size() != len(encodings)) {
        cerr << "Failed to load faces" << endl;
        return -1;
    }
    
    string folderPath = "Resources/";
    Mat bg = imread(folderPath + "Background.png");
    std::array<Mat, 4> sidebars;
    
    for (int i = 0; i < 4; i++) {
        sidebars[i] = imread(folderPath + "sidebar" + to_string(i) + ".png");
        if (sidebars[i].empty()) {
            cerr << "Failed to load sidebar " << i << endl;
            return -1;
        }
    }
    
    CascadeClassifier faceCascade;
    
    faceCascade.load("Models/haarcascade_frontalface_default.xml");
    
    if (faceCascade.empty()) {
        cerr << "Cascade Classifier failed to load" << endl;
        return -1;
    }
    
    Mat img;
    VideoCapture cap(0);
    Rect roi(102, 330, 960, 540);
    Rect sidebarRoi(1164, 0, 756, 1080);
    Rect faceRoi(1362, 265.9, 360, 360);
    
    int modeType = 0;
    std::array<int, 4> buffers = {0, 0, 0, 0};
    
    std::unordered_map<string, string> dict = {
        {"name", "placeholder"},
        {"seat", "placeholder"},
        {"class", "placeholder"}
    };
    
    while (true) {
        cap.read(img);
        resize(img, img, Size(960, 540));
        
        sidebars[modeType].copyTo(bg(sidebarRoi));
        
        buffers[modeType] += 1;
        
        if (buffers[1] == 120) {
            buffers[1] = 0;
            modeType = 3;
        }
        else if (buffers[2] == 120) {
            buffers[2] = 0;
            modeType = 0;
        }
        else if (buffers[3] == 120) {
            buffers[3] = 0;
            modeType = 0;
        }
        
        // Render data
        if (modeType == 3) {
            Mat face = imread("Images/" + dict["id"] + ".png");
            resize(face, face, Size(360, 360));
            face.copyTo(bg(faceRoi));
            
            Size textSize = getTextSize(dict["name"], FONT_HERSHEY_SIMPLEX, 2.0, 3, 0);
            
            Point center(1542, 700);

            Point textOrg(center.x - textSize.width / 2,
                          center.y + textSize.height / 2);
            
            putText(bg, dict["name"], textOrg, FONT_HERSHEY_SIMPLEX,
                    2.0, Scalar(0, 0, 0), 5);
            putText(bg, dict["seat"], Point(1532, 814), FONT_HERSHEY_SIMPLEX,
                    1.5, Scalar(255, 255, 255), 3);
            putText(bg, dict["class"], Point(1532, 939), FONT_HERSHEY_SIMPLEX,
                    1.5, Scalar(255, 255, 255), 3);
        }
        
        vector<Rect> faces;
        
        // Detects faces once per second (30fps webcam)
        if (buffers[0] == 30) {
            buffers[0] = 0;
            faceCascade.detectMultiScale(img, faces, 1.1, 10);
        }
        
        if (faces.size() > 1) {
            cout << "One person in frame at a time" << endl;
        }
        else if (!faces.empty()) {
            cv::Mat rgb = img.clone();
            cv::cvtColor(rgb, rgb, cv::COLOR_BGR2RGB);
            
            cout << "Face found" << endl;
            
            // Build known face location
            auto face = faces[0];
            int top    = face.y;
            int left   = face.x;
            int bottom = face.y + face.height;
            int right  = face.x + face.width;
            
            py::list py_locations;
            py_locations.append(py::make_tuple(top, right, bottom, left));

            py::array np_rgb = mat_to_numpy(rgb);

            // Ask face_recognition to encode using the provided locations
            py::list enc_list = fr.attr("face_encodings")(np_rgb, py_locations);
            py::object first_enc = enc_list[0];
            
            auto matches = fr.attr("compare_faces")(encodings, first_enc);
                            
            if (np.attr("sum")(matches).cast<int>() > 0) {
                auto face_dist = fr.attr("face_distance")(encodings, first_enc);
                int matchIndex = np.attr("argmin")(face_dist).cast<int>();
                string id = ids[matchIndex];
                string query = "SELECT * FROM public.boarding_system WHERE id=" + id;
                
                pqxx::work tx{*c};
                auto entry = tx.exec(query);
                for (auto const& e: entry) {
                    dict["id"] = e["id"].as<string>();
                    dict["name"] = e["name"].as<string>();
                    dict["seat"] = e["seat"].as<string>();
                    dict["class"] = e["class"].as<string>();
                }
                modeType = 1;
            }
            else {
                cout << "Not recognized" << endl;
                modeType = 2;
            }
        }
        
        img.copyTo(bg(roi));
                
        imshow("Webcam", bg);
        if (waitKey(1) >= 0) break;
    }
    
    return 0;
}
