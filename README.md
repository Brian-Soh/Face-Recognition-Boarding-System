# Face Recognition Boarding System

## Introduction
In this project I created an airport terminal boarding system which operates under the alias Air Zenith. The boarding system leverages a Haar Cascade Classifier for quick face detection and passes the frame into a Res-Net based convolutional neural network (CNN) which creates a 128-dimension embedding of it. The embedding is them compared to a database of passengers and if a match is found, the passenger's boarding pass data is loaded from an AWS Relational Database Service (RDS) (Postgres) and they are marked present.
