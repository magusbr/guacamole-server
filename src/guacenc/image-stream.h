/*
 * Copyright (C) 2016 Glyptodon, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef GUACENC_IMAGE_STREAM_H
#define GUACENC_IMAGE_STREAM_H

#include "config.h"
#include "buffer.h"

/**
 * A decoder implementation which processes arbitrary image data of a
 * particular type. Image data is fed explicitly into the decoder as chunks.
 */
typedef struct guacenc_decoder guacenc_decoder;

/**
 * The current state of an allocated Guacamole image stream.
 */
typedef struct guacenc_image_stream {

    /**
     * The index of the destination layer or buffer.
     */
    int index;

    /**
     * The Guacamole protocol compositing operation (channel mask) to apply
     * when drawing the image.
     */
    int mask;

    /**
     * The X coordinate of the upper-left corner of the rectangle within the
     * destination layer or buffer that the decoded image should be drawn to.
     */
    int x;

    /**
     * The Y coordinate of the upper-left corner of the rectangle within the
     * destination layer or buffer that the decoded image should be drawn to.
     */
    int y;

    /**
     * The decoder to use when decoding the raw data received along this
     * stream, or NULL if no such decoder exists.
     */
    guacenc_decoder* decoder;

    /**
     * Arbitrary implementation-specific data associated with the stream.
     */
    void* data;

} guacenc_image_stream;

/**
 * Callback function which is invoked when a decoder has been assigned to an
 * image stream.
 *
 * @param stream
 *     The image stream that the decoder has been assigned to.
 *
 * @return
 *     Zero if initialization was successful, non-zero otherwise.
 */
typedef int guacenc_decoder_init_handler(guacenc_image_stream* stream);

/**
 * Callback function which is invoked when data has been received along an
 * image stream with an associated decoder.
 *
 * @param stream
 *     The image stream that the decoder was assigned to.
 *
 * @param data
 *     The chunk of data received along the image stream.
 *
 * @param length
 *     The length of the chunk of data received, in bytes.
 *
 * @return
 *     Zero if the provided data was processed successfully, non-zero
 *     otherwise.
 */
typedef int guacenc_decoder_data_handler(guacenc_image_stream* stream,
        unsigned char* data, int length);

/**
 * Callback function which is invoked when an image stream with an associated
 * decoder has ended (reached end-of-stream). The image stream will contain
 * the required meta-information describing the drawing operation, including
 * the destination X/Y coordinates.
 *
 * @param stream
 *     The image stream that has ended.
 *
 * @param buffer
 *     The buffer to which the decoded image should be drawn.
 *
 * @return
 *     Zero if the end of the stream has been processed successfully and the
 *     resulting image has been rendered to the given buffer, non-zero
 *     otherwise.
 */
typedef int guacenc_decoder_end_handler(guacenc_image_stream* stream,
        guacenc_buffer* buffer);

/**
 * Callback function which will be invoked when the data associated with an
 * image stream must be freed. This may happen at any time, and will not
 * necessarily occur only after the image stream has ended. It is possible
 * that an image stream will be in-progress at the end of a protocol dump, thus
 * the memory associated with the stream will need to be freed without ever
 * ending.
 *
 * @param stream
 *     The stream whose associated data must be freed.
 *
 * @return
 *     Zero if the data was successfully freed, non-zero otherwise.
 */
typedef int guacenc_decoder_free_handler(guacenc_image_stream* stream);

struct guacenc_decoder {

    /**
     * Callback invoked when this decoder has just been assigned to an image
     * stream.
     */
    guacenc_decoder_init_handler* init_handler;

    /**
     * Callback invoked when data has been received along an image stream to
     * which this decoder has been assigned.
     */
    guacenc_decoder_data_handler* data_handler;

    /**
     * Callback invoked when an image stream to which this decoder has been
     * assigned has ended (reached end-of-stream).
     */
    guacenc_decoder_end_handler* end_handler;

    /**
     * Callback invoked when data associated with an image stream by this
     * decoder must be freed.
     */
    guacenc_decoder_free_handler* free_handler;

};

/**
 * Mapping of image mimetype to corresponding decoder.
 */
typedef struct guacenc_decoder_mapping {

    /**
     * The mimetype of the image that the associated decoder can read.
     */
    const char* mimetype;

    /**
     * The decoder to use when an image stream of the associated mimetype is
     * received.
     */
    guacenc_decoder* decoder;

} guacenc_decoder_mapping;

/**
 * Array of all mimetype/decoder mappings for all supported image types,
 * terminated by an entry with a NULL mimetype.
 */
extern guacenc_decoder_mapping guacenc_decoder_map[];

/**
 * Returns the decoder associated with the given mimetype. If no such decoder
 * exists, NULL is returned.
 *
 * @param mimetype
 *     The image mimetype to return the associated decoder of.
 *
 * @return
 *     The decoder associated with the given mimetype, or NULL if no such
 *     decoder exists.
 */
guacenc_decoder* guacenc_get_decoder(const char* mimetype);

/**
 * Allocates and initializes a new image stream. This allocation is independent
 * of the Guacamole video encoder display; the allocated guacenc_image_stream
 * will not automatically be associated with the active display, nor will the
 * provided layer/buffer index be validated.
 *
 * @param mask
 *     The Guacamole protocol compositing operation (channel mask) to apply
 *     when drawing the image.
 *
 * @param index
 *     The index of the layer or bugger that the image should be drawn to.
 *
 * @param mimetype
 *     The mimetype of the image data that will be received along this stream.
 *
 * @param x
 *     The X coordinate of the upper-left corner of the rectangle within the
 *     destination layer or buffer that the image should be drawn to.
 *
 * @param y
 *     The Y coordinate of the upper-left corner of the rectangle within the
 *     destination layer or buffer that the image should be drawn to.
 *
 * @return
 *     A newly-allocated and initialized guacenc_image_stream, or NULL if
 *     allocation fails.
 */
guacenc_image_stream* guacenc_image_stream_alloc(int mask, int index,
        const char* mimetype, int x, int y);

/**
 * Signals the decoder of the given image stream that a chunk of image data
 * has been received. If no decoder is associated with the given image stream,
 * this function has no effect.
 *
 * @param stream
 *     The image stream that received the data.
 *
 * @param data
 *     The chunk of data received along the image stream.
 *
 * @param length
 *     The length of the chunk of data received, in bytes.
 *
 * @return
 *     Zero if the given data was handled successfully by the decoder, or
 *     non-zero if an error occurs.
 */
int guacenc_image_stream_receive(guacenc_image_stream* stream,
        unsigned char* data, int length);

/**
 * Signals the decoder of the given image stream that no more data will be
 * received and the image should be written to the given buffer as-is. If no
 * decoder is associated with the given image stream, this function has no
 * effect. Meta-information describing the image draw operation itself is
 * stored within the guacenc_image_stream.
 *
 * @param stream
 *     The image stream that has ended.
 *
 * @param buffer
 *     The buffer that the decoded image should be written to.
 *
 * @return
 *     Zero if the image is written successfully, or non-zero if an error
 *     occurs.
 */
int guacenc_image_stream_end(guacenc_image_stream* stream,
        guacenc_buffer* buffer);

/**
 * Frees the given image stream and all associated data. If the image stream
 * has not yet ended (reached end-of-stream), no image will be drawn to the
 * associated buffer or layer.
 *
 * @param stream
 *     The stream to free.
 *
 * @return
 *     Zero if freeing the stream succeeded, or non-zero if freeing the stream
 *     failed (for example, due to an error in the free handler of the
 *     decoder).
 */
int guacenc_image_stream_free(guacenc_image_stream* stream);

#endif

