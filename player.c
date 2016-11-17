#include "player.h"

static void
registerDiscoverer()
{
    CustomData *data;
    GError *err = NULL;
    gchar *url = "http::/docs.gstreamer.com/media/sintel_trailer-480p.webm";

    /* initialize custom data structure */
    memset(&data, 0, sizeof(data));
}

static void
handleCompleteEvent(gstData *data)
{
    g_print(" [handleCompleteEvent]\n");
    stop_playback_pipe(data);
    return;
}

static gboolean
handlePlaybinEvent(GstBus *bus, GstMessage *message, gpointer data)
{
    GstObject *src = GST_MESSAGE_SRC(message);
    GMainLoop *loop = data;

    g_print(" [handlePlaybinEvent] : %s\n", gst_object_get_name(src));

    if (src != (GstObject *)gdata.pipeline)
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
            handleCompleteEvent(&gdata);
            g_main_loop_quit(loop);
            break;
        case GST_MESSAGE_STATE_CHANGED:
            break;
        default:
            break;
    }

    return TRUE;
}

static void
on_pad_added(GstElement *src_element, GstPad *src_pad, gpointer data)
{
    // TODO
}

static gboolean
create_pipeline(gstData *data)
{
    /* Creating a new pipeline */
    data->pipeline= gst_pipeline_new("pipeline");
    if (data->pipeline == NULL) {
        return FALSE;
    }
    gst_element_set_state(data->pipeline, GST_STATE_NULL);
    return TRUE;
}

static gboolean
add_bin_playback_to_pipe(gstData *data)
{
    if ((gst_bin_add(GST_BIN(data->pipeline), data->bin_element)) != TRUE) {
        g_print("Fail to add bin to pipeline.\n");
        return FALSE;
    }

    if (gst_element_set_state(data->pipeline, GST_STATE_NULL) == GST_STATE_CHANGE_SUCCESS) {
        return TRUE;
    } else {
        g_print("Fail to set pipeline state to NULL.\n");
        return FALSE;
    }
}

static gboolean
init_audio_playback_pipeline(gstData *data)
{
    if (data == NULL) {
        return FALSE;
    }

    /* Creating a source element */
    data->source_element = gst_element_factory_make("filesrc", "source");

    /**
     * Creating a de-coder element
     *   You are linking against GStreamer-1.0 "decodebin2" was renamed
     *       to "decodebin" in GStreamer-1.0.
     * gst_element_factory_make ("decodebin2",  "decoder");
     * gst_element_factory_make("decodebin", "decoder");
     */
    if (strstr(data->filelocation, ".mp3")) {
        g_print("MP3 Audio decoder selected.\n");
        data->decoder_element = gst_element_factory_make("mad", "mp3decoder");
    }

    if (strstr(data->filelocation, ".wav")) {
        g_print("WAV Audio decoder selected.\n");
        data->decoder_element = gst_element_factory_make("wavparse", "wavdecoer");
    }

    /* Creating a new audio-converter element */
    data->converter_element = gst_element_factory_make("audioconvert", "audioconverter");

    /**
     *  Creating a new sink element
     *      autoaudiosink
     *      alsasink
     */
    data->sink_element = gst_element_factory_make("alsasink", "play_audio");

    if (!data->source_element || !data->decoder_element || !data->converter_element || !data->sink_element) {
        g_print("Faile to create all of elements.\n");
        return FALSE;
    }

    /* Set a audio file to source element. */
    g_object_set(G_OBJECT(data->source_element), "location", data->filelocation, NULL);

    /* Creating a bin */
    data->bin_element = gst_bin_new("bin");

    /* Linking src to sink element */
    if (strstr(data->filelocation, ".mp3")) {
        gst_bin_add_many(GST_BIN(data->bin_element), data->source_element, data->decoder_element, data->converter_element, data->sink_element, NULL);

        if (gst_element_link_many (data->source_element, data->decoder_element, NULL) != TRUE) {
            g_print("Fail to link source with decoder.\n");
        }

        if (gst_element_link_many (data->decoder_element, data->converter_element, NULL) != TRUE) {
            g_print("Fail to link decoder with converter.\n");
        }

        if (gst_element_link_many (data->converter_element, data->sink_element, NULL) != TRUE) {
            g_print("Fail to link a converter with sink.\n");
        }
    }

    if (strstr(data->filelocation, ".wav")) {
        gst_bin_add_many(GST_BIN(data->bin_element), data->source_element, data->decoder_element, data->sink_element,NULL);

        if (gst_element_link_many (data->source_element, data->decoder_element, NULL) != TRUE) {
            g_print("Fail to link source with decoder.\n");
        }

        if (gst_element_link_many (data->decoder_element, data->sink_element, NULL) != TRUE) {
            g_print("Fail to link decoder with sink.\n");
        }

        g_signal_connect(data->decoder_element, "pad-added", G_CALLBACK(on_pad_added), data->sink_element);
    }
    g_print("Done to link src to sink element.\n");

    return TRUE;
        
}

static gboolean
start_playback_pipe(gstData *data)
{
    /* Creating a main loop*/
    data->loop = g_main_loop_new(NULL, FALSE);

    /* Creating a bus */
    data->bus = gst_pipeline_get_bus(GST_PIPELINE(data->pipeline));
    gst_bus_add_watch(data->bus, handlePlaybinEvent, data->loop);
    g_print("Done to create a bus\n");

    /* Start playing */
    gst_element_set_state (GST_ELEMENT(data->pipeline), GST_STATE_PLAYING);
    g_main_loop_run(data->loop);
    //state = gst_element_get_state(data->pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
    //g_print("Start state = %d\n", state);
    return TRUE;
}

static void
stop_playback_pipe(gstData *data)
{
    g_print("Stop playback\n");

    /* Stop playing */
    gst_element_set_state (GST_ELEMENT(data->pipeline), GST_STATE_NULL);
    gst_element_get_state(data->pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
}

/* un-referencing all the elements in the pipeline */
static void
delete_pipeline(gstData *data)
{
    if (data->pipeline) {
        gst_element_set_state(data->pipeline, GST_STATE_NULL);
    }

    if (data->bus) {
        gst_object_unref(GST_OBJECT(data->bus));
    }

    if (data->pipeline) {
        gst_object_unref(GST_OBJECT(data->pipeline));
    }
}

void play(const char *path)
{
    g_print("play()\n");

    gst_init(NULL, NULL);

    memset(gdata.filelocation, 0, sizeof(gdata.filelocation));
    strcpy(gdata.filelocation, path);

    if (!create_pipeline(&gdata)) {
        // TODO
    }

    if (init_audio_playback_pipeline(&gdata)) {
        if (!add_bin_playback_to_pipe(&gdata)) {
            // TODO
        }
        if (start_playback_pipe(&gdata)) {
            // TODO 
        }
    }

    delete_pipeline(&gdata);

    return;
}
