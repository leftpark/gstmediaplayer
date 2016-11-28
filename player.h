#include <stdio.h>
#include <string.h>

#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>    /* GstDiscoverer */

#include "scanner.h"

#define DUMMY_MP3_FILE "dummy.mp3"
#define DUMMY_OGG_FILE "dummy.ogg"

#define LOG(format, args...)\
        printf(format " %s(%d) : ", __FILE__, __LINE__, ##args)

/**
 * Structure to contain all our information,
 * so we can pass it around.
 */
typedef struct _CustomData {
    GstDiscoverer *discoverer;
    GMainLoop   *loop;
} CustomData;

typedef struct
{
    GstElement *source_element;     // filesrc
    GstElement *decoder_element;    // decoder
    GstElement *converter_element;  // audioconvert 
    GstElement *sink_element;       // audiosink
    GstElement *bin_element;        // bin
    
    GstStateChangeReturn state;
    GMainLoop *loop;

    GstElement *pipeline;
    GstBus *bus;

    gchar filelocation[256];
}gstData; 

gstData gdata;

static void registerDiscoverer(void);

static void handleCompleteEvent(gstData *data);
static gboolean handlePlaybinEvent(GstBus *bus, GstMessage *message, gpointer data);

static void on_pad_added(GstElement *src_element, GstPad *src_pad, gpointer data);
static gboolean create_pipeline(gstData *data);
static gboolean add_bin_playback_to_pipe(gstData *dat);
static gboolean init_audio_playback_pipeline(gstData *data);
static gboolean start_playback_pipe(gstData *data);
static void stop_playback_pipe(gstData *data);
static void delete_pipeline(gstData *data);
void play(const char *path);
void stop();
