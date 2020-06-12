#!/bin/env python3

import sys
import cv2 as cv
import numpy as np

def sigmoid(X):
   return 1/(1+np.exp(-X))

def main(cam_num, net):
    cam = cv.VideoCapture(int(cam_num))
    net = cv.dnn.readNetFromTensorflow(net)
    while cv.waitKey(1) < 0:
        _, img = cam.read()
        imgHeight, imgWidth, _ = img.shape
        in1 = cv.dnn.blobFromImage(img, 1.0, swapRB=True)
        # Normalize input for mobilenet requirements
        in1 = (in1/127.5)-1
        net.setInput(in1)
        out = net.forward("float_segments/conv")
        out_imgs = cv.dnn.imagesFromBlob(out)
        segments = out_imgs[0]
        segments = sigmoid(segments)
        segments[segments > 0.7] = 255
        segments[segments <= 0.7] = 0
        t, _ = net.getPerfProfile()
        freq = cv.getTickFrequency() / 1000
        print('%.2fms' % (t / freq))
        cv.imshow('frame', segments.astype('uint8'))

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: bodypix.py <video device number> <network>")
        sys.exit(1)
    main(sys.argv[1], sys.argv[2])
