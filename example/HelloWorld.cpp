/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "example/HelloWorld.h"
#include <iostream>

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkShader.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkGradientShader.h"
#include "include/encode/SkPngEncoder.h"
#include "tools/window/DisplayParams.h"

#include <string.h>

using namespace sk_app;
using skwindow::DisplayParams;

Application* Application::Create(int argc, char** argv, void* platformData) {
    return new HelloWorld(argc, argv, platformData);
}

HelloWorld::HelloWorld(int argc, char** argv, void* platformData)
#if defined(SK_GL) && defined(FOOOOOOO)
        : fBackendType(Window::kNativeGL_BackendType)
        ,
#elif defined(SK_VULKAN)
        : fBackendType(Window::kVulkan_BackendType)
        ,
#else
        : fBackendType(Window::kRaster_BackendType)
        ,
#endif
        fRotationAngle(0)
        , saveScreenshot(true) {
    SkGraphics::Init();

    fWindow = Window::CreateNativeWindow(platformData);
    fWindow->setRequestedDisplayParams(DisplayParams());

    // register callbacks
    fWindow->pushLayer(this);

    fWindow->attach(fBackendType);
}

HelloWorld::~HelloWorld() {
    fWindow->detach();
    delete fWindow;
}

void HelloWorld::updateTitle() {
    if (!fWindow) {
        return;
    }

    SkString title("Hello World ");
    if (Window::kRaster_BackendType == fBackendType) {
        title.append("Raster");
    } else {
#if defined(SK_GL)
        title.append("GL");
#elif defined(SK_VULKAN)
        title.append("Vulkan");
#elif defined(SK_DAWN)
        title.append("Dawn");
#else
        title.append("Unknown GPU backend");
#endif
    }

    fWindow->setTitle(title.c_str());
}

void HelloWorld::onBackendCreated() {
    this->updateTitle();
    fWindow->show();
    fWindow->inval();
}

void HelloWorld::onPaint(SkSurface* surface) {
    auto canvas = surface->getCanvas();

    for (int i = 0; i < 1; i++) {
        // Clear background
        canvas->clear(SK_ColorWHITE);

        SkPaint paint;
        paint.setColor(SK_ColorRED);

        // Draw a rectangle with red paint
        SkRect rect = SkRect::MakeXYWH(10, 10, 128, 128);
        canvas->drawRect(rect, paint);

        // Set up a linear gradient and draw a circle
        {
            SkPoint linearPoints[] = {{0, 0}, {300, 300}};
            SkColor linearColors[] = {SK_ColorGREEN, SK_ColorBLACK};
            paint.setShader(SkGradientShader::MakeLinear(
                    linearPoints, linearColors, nullptr, 2, SkTileMode::kMirror));
            paint.setAntiAlias(true);

            canvas->drawCircle(200, 200, 64, paint);

            // Detach shader
            paint.setShader(nullptr);
        }

        // Draw a message with a nice black paint
        SkFont font;
        font.setSubpixel(true);
        font.setSize(20);
        paint.setColor(SK_ColorBLACK);

        canvas->save();
        static char message[250];
        sprintf(message, "Hello World %d", i);

        // Translate and rotate
        // canvas->translate(300, 300);
        fRotationAngle += 0.2f;
        if (fRotationAngle > 360) {
            fRotationAngle -= 360;
        }
        canvas->rotate(fRotationAngle);

        // Draw the text
        canvas->drawSimpleText(message, strlen(message), SkTextEncoding::kUTF8, 0, 0, font, paint);

        canvas->restore();

        // save to PNG
        auto filename = "helloworld-" + std::to_string(i) + ".png";

        if (saveScreenshot) {
            // save the surface to a PNG file
            SkPixmap pixmap;
            surface->peekPixels(&pixmap);

            SkFILEWStream out(filename.c_str());
            if (!out.isValid()) {
                std::cerr << "Could not open file for writing.\n";
                return;
            }

            // Use default encoding options.
            SkPngEncoder::Options png_options;

            if (!SkPngEncoder::Encode(&out, pixmap, png_options)) {
                std::cerr << "PNG encoding failed.\n";
            }
        }
    }
    saveScreenshot = false;

    // exit the program
    exit(0);
}

void HelloWorld::onIdle() {
    // Just re-paint continuously
    fWindow->inval();
}

bool HelloWorld::onChar(SkUnichar c, skui::ModifierKey modifiers) {
    if (' ' == c) {
        if (Window::kRaster_BackendType == fBackendType) {
#if defined(SK_GL)
            fBackendType = Window::kNativeGL_BackendType;
#elif defined(SK_VULKAN)
            fBackendType = Window::kVulkan_BackendType;
#else
            SkDebugf("No GPU backend configured\n");
            return true;
#endif
        } else {
            fBackendType = Window::kRaster_BackendType;
        }
        fWindow->detach();
        fWindow->attach(fBackendType);
    } else if ('s' == c) {
        saveScreenshot = true;
    }
    return true;
}
