#ifndef PONG_CONFIG_H
#define PONG_CONFIG_H

// Game Modes
//#define PONG_TEST_MODE

// Game Data
#define NUM_PLAYERS 2

// Webcam Data
#define PONG_IMG_WIDTH_PX 640
#define PONG_IMG_HEIGHT_PX 480
#define PONG_IMG_SIZE (PONG_IMG_WIDTH_PX * PONG_IMG_HEIGHT_PX)

// Networking
#define REMOTE_HOST "127.0.0.1"
#define VIDEO_PORT 5200
#define SERVER_RX_PORT VIDEO_PORT
#define SERVER_TX_PORT VIDEO_PORT + 1

#endif
