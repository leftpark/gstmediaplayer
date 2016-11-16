#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>    /* GstDiscoverer */
#include <string.h>
#include "player.h"

#define DUMMY_MP3_FILE "dummy.mp3"
#define DUMMY_OGG_FILE "dummy.ogg"

/**
 * Structure to contain all our information,
 * so we can pass it around.
 */
typedef struct _CustomData {
    GstDiscoverer *discoverer;
    GMainLoop   *loop;
} CustomData;

GstElement *mPipeline;
GstBus *mBus;

void
registerDiscoverer()
{
    CustomData *data;
    GError *err = NULL;
    gchar *url = "http::/docs.gstreamer.com/media/sintel_trailer-480p.webm";

    /* initialize custom data structure */
    memset(&data, 0, sizeof(data));


}

void
handleCompleteEvent()
{
    g_print(" [handleCompleteEvent]\n");
    return;
}

gboolean
handlePlaybinEvent(GstBus *bus, GstMessage *message, gpointer data)
{
    GstObject *src = GST_MESSAGE_SRC(message);
    GMainLoop *loop = data;

    g_print(" [handlePlaybinEvent] : %s\n", gst_object_get_name(src));

    if (src != (GstObject *)mPipeline)
    {
        return TRUE;
    }

    g_print(" [handlePlaybinEvent] : bus(%p), name(%s), msg(%s)\n",
            bus,
            gst_object_get_name(src),
            GST_MESSAGE_TYPE_NAME(message));

    switch(GST_MESSAGE_TYPE(message))
    {
        case GST_MESSAGE_ERROR:
        {
            GError *err;
            gchar *debug;

            gst_message_parse_error (message, &err, &debug);
            g_print(" [handlePlaybinEvent] : Error: %s\n", err->message);
            g_error_free(err);

            g_main_loop_quit(loop);
            break;
        }
        case GST_MESSAGE_EOS:
            g_print(" [handlePlaybinEvent] : End-Of-Stream reached.\n");
            handleCompleteEvent();
            g_main_loop_quit(loop);
            break;
        case GST_MESSAGE_STATE_CHANGED:
            break;
        default:
            break;
    }

    return TRUE;
}

void play(const char *path)
{
    g_print("play()\n");

    GstElement *source_element; //filesrc
    GstElement *decoder_element; // decoder
    GstElement *converter_element; // audioconvert 
    GstElement *sink_element; // audiosink
    //GstElement *pipeline;
    GstStateChangeReturn state;
    GMainLoop *loop;

    gst_init(NULL, NULL);

    /* Creating a main loop*/
    loop = g_main_loop_new(NULL, FALSE);

    /* Creating a new pipeline */
    mPipeline = gst_pipeline_new("pipeline");

    /* Creating a source element */
    source_element = gst_element_factory_make("filesrc", "source");
    if (source_element == NULL)
    {
        g_print("Failed to create a source element.\n");
        return;
    }

    //g_object_set(G_OBJECT(source_element), "location", DUMMY_MP3_FILE, NULL);
    g_object_set(G_OBJECT(source_element), "location", path, NULL);

    /**
     * Creating a de-coder element
     *   You are linking against GStreamer-1.0 "decodebin2" was renamed
     *       to "decodebin" in GStreamer-1.0.
     * decoder_element = gst_element_factory_make ("decodebin2",  "decoder");
     * decoder_element = gst_element_factory_make("decodebin", "decoder");
     *
     * Using "mad" doecoder For mp3 file
     * decoder_element = gst_element_factory_make("mad", "decoder");
     */
    decoder_element = gst_element_factory_make("mad", "decoder");
    if (decoder_element == NULL)
    {
        g_print("Failed to create a decoder element.\n");
        return; 
    }

    if (strstr(path, ".mp3")) {
        /* Creating a new audio-converter element */
        converter_element = gst_element_factory_make("audioconvert", "audioconverter");
        if (converter_element == NULL)
        {
            g_print("Failed to create a audio-converter element.\n");
            return;
        }
    }

    /**
     *  Creating a new sink element
     *      autoaudiosink
     *      alsasink
     */
    sink_element = gst_element_factory_make("alsasink", "play_audio");
    if (sink_element == NULL) {
        g_print("Failed to create a sink element.\n");
        return;
    }

    /* Adding elements to pipeline */
    gst_bin_add_many(GST_BIN(mPipeline), source_element, decoder_element, converter_element, sink_element, NULL);

    /* Creating a bus */
    mBus = gst_pipeline_get_bus(GST_PIPELINE(mPipeline));
    gst_bus_add_watch(mBus, handlePlaybinEvent, loop);
    g_print("Done to create a bus\n");

    /* Linking src to sink element */
    gst_element_link_many (source_element, decoder_element, NULL);
    gst_element_link_many (decoder_element, converter_element, NULL);
    gst_element_link_many (converter_element, sink_element, NULL);
    g_print("Done to link src to sink element\n");

    /* Start playing */
    gst_element_set_state (GST_ELEMENT(mPipeline), GST_STATE_PLAYING);
    g_main_loop_run(loop);
    //state = gst_element_get_state(mPipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
    g_print("Start state = %d\n", state);

    /* Stop playing */
    gst_element_set_state (GST_ELEMENT(mPipeline), GST_STATE_NULL);
    gst_element_get_state(mPipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
    g_print("Stop state = %d\n", state);

    /* un-referencing all the elements in the pipeline */
    gst_object_unref(GST_OBJECT(mPipeline));
    gst_object_unref(GST_OBJECT(mBus));

    return;
}
