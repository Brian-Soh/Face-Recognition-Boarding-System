# Face Recognition Boarding System

## Introduction
In this project I created an airport terminal boarding system which operates under the alias Air Zenith. The boarding system leverages a Haar Cascade classifier for quick face detection and passes the frame into a Res-Net based convolutional neural network (CNN) which creates a 128-dimension embedding of it. The embedding is them compared to a database of passengers and if a match is found, the passenger's boarding pass data is loaded from an AWS Relational Database Service (RDS) (Postgres) and they are marked present. Check out a demo [here](https://www.youtube.com/watch?v=6IHd2AIZVbw)!

## Tree View of Project

```text
Face-Recognition-Boarding-System/ 
├── Images/ 
│   ├── 192894.png (Elon Musk) 
│   ├── 390439.png (Jeff Bezos)
│   └── 490303.png (Mark Zuckerberg)
├── Models/
│   └── haarcascade_frontalface_default.xml
└── Resources/
│   ├── background.png
│   ├── sidebar0.png
│   ├── sidebar1.png
│   ├── sidebar2.png
│   └── us-west-2-bundle.pem
└── src/
    ├── main.h
    ├── main.cpp
    └── insertData.cpp
```

The Haar Cascade classifier can be found in the `Models` folder while each component of the GUI and the AWS RDS pem key (public key which verifies server) can be found in the /Resources. 

## Sample Run
I created a data pipeline via libpqxx in the file `insertData.cpp` which uploads data for 3 passengers; in this case: Elon Musk, Jeff Bezos, and Mark Zuckerberg. In `main.cpp`, I created a face encoding pipeline to prepare the passenger database by creating 128-dimension embeddings of the photos in /Images. THen I load /Images/background.png into an openCV Mat object which serves as the canvas for all subsequent overlays such as the sidebars switching between real-time face detection, passenger information display, and system feedback.

Afterward, I use an OpenCV VideoCapture object to continually stream from the webcam. Every second, a frame is passed into the Haar Cascade classifier. If a face is detected, then I use Python's face_recognition library via pybind11 to create a 128-dimension embedding of it and compare it to the passenger database. If no match is found, the sidebar will switch modes to show that and wait 4 seconds before searching for another face. If a match is found, then the closest match via the precomputed embeddings is used and its ID is queried from AWS RDS (Postgres) via libqpxx. The passenger data along with the sidebar and webcam is then rendered onto the dedicated background layer.

I am working on integrating s3 buckets in this pipeline as well so that "ID" photos of the passengers can be queried and rendered in the GUI.






