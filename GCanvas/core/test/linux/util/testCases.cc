#include <unordered_map>
#include <functional>
#include <string>
#include <math.h>
#include "GCanvas.hpp"
void prepareCases(std::unordered_map<std::string, std::function<void(std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *mCanvasContext, int width, int height)>> &testCases)
{
    testCases["tc_2d_arc"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->SetFillStyle("#000000 ");
        ctx->FillRect(0, 0, width, height);
        ctx->BeginPath();
        ctx->SetStrokeStyle("yellow");
        ctx->Arc(50 * ratio, 75 * ratio, 50 * ratio, 0, 0.5 * M_PI);
        ctx->Stroke();
    };
    testCases["tc_2d_arcTo"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->SetFillStyle("#000000 ");
        ctx->FillRect(0, 0, width, height);
        ctx->BeginPath();
        ctx->SetStrokeStyle("yellow");
        ctx->MoveTo(20 * ratio, 20 * ratio);                                      // 创建开始点
        ctx->LineTo(100 * ratio, 20 * ratio);                                     // 创建水平线
        ctx->ArcTo(150 * ratio, 20 * ratio, 150 * ratio, 70 * ratio, 50 * ratio); // 创建弧
        ctx->LineTo(150 * ratio, 120 * ratio);                                    // 创建垂直线
        ctx->Stroke();
    };
    testCases["tc_2d_MDN_arc"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        for (int i = 0; i <= 3; i++)
        {
            for (int j = 0; j <= 2; j++)
            {
                ctx->BeginPath();
                int x = 25 + j * 50 * ratio;          // x coordinate
                int y = 25 + i * 50 * ratio;          // y coordinate
                int radius = 20 * ratio;              // Arc radius
                int startAngle = 0;                   // Starting point on circle
                int endAngle = M_PI + (M_PI * j) / 2; // End point on circle
                int anticlockwise = i % 2 == 1;       // Draw anticlockwise

                ctx->Arc(x, y, radius, startAngle, endAngle, anticlockwise);

                if (i > 1)
                {
                    ctx->Fill();
                }
                else
                {
                    ctx->Stroke();
                }
            }
        }
    };
    testCases["tc_2d_MDN_arcTo_1"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        // Tangential lines
        ctx->BeginPath();
        ctx->SetStrokeStyle("gray");
        ctx->MoveTo(200 * ratio, 20 * ratio);
        ctx->LineTo(200 * ratio, 130 * ratio);
        ctx->LineTo(50 * ratio, 20 * ratio);
        ctx->Stroke();

        // Arc
        ctx->BeginPath();
        ctx->SetStrokeStyle("black");
        ctx->SetLineWidth(5 * ratio);
        ctx->MoveTo(200 * ratio, 20 * ratio);
        ctx->ArcTo(200 * ratio, 130 * ratio, 50 * ratio, 20 * ratio, 40 * ratio);
        ctx->Stroke();

        // Start point
        ctx->BeginPath();
        ctx->SetFillStyle("blue");
        ctx->Arc(200 * ratio, 20 * ratio, 5 * ratio, 0, 2 * M_PI);
        ctx->Fill();

        // Control points
        ctx->BeginPath();
        ctx->SetFillStyle("red");
        ctx->Arc(200 * ratio, 130 * ratio, 5 * ratio, 0, 2 * M_PI); // Control point one
        ctx->Arc(50 * ratio, 20 * ratio, 5 * ratio, 0, 2 * M_PI);   // Control point two
        ctx->Fill();
    };
    testCases["tc_2d_MDN_arcTo_2"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->BeginPath();
        ctx->MoveTo(180 * ratio, 90 * ratio);
        ctx->ArcTo(180 * ratio, 130 * ratio, 110 * ratio, 130 * ratio, 130 * ratio);
        ctx->LineTo(110 * ratio, 130 * ratio);
        ctx->Stroke();
    };
    testCases["td_2d_clip"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->SetFillStyle("#000000");
        ctx->FillRect(0, 0, width, height);
        ctx->SetFillStyle("#ff0000");
        ctx->SetStrokeStyle("#fff");

        ctx->BeginPath();
        ctx->Arc(150 * ratio, 150 * ratio, 75 * ratio, 0 * ratio, 3.14 * 2, false);
        ctx->Clip();

        ctx->FillRect(0, 0, 200 * ratio, 200 * ratio);
        ctx->SetFillStyle("#00ff00");
        ctx->FillRect(180 * ratio, 180 * ratio, 200 * ratio, 200 * ratio);
    };

    testCases["tc_2d_clearRect"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->ClearRect(0, 0, width, height);
    };
    testCases["tc_2d_fill"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;

        ctx->SetFillStyle("#000000");
        ctx->FillRect(0, 0, width, height);

        ctx->SetFillStyle("#FFF");
        ctx->SetLineWidth(2);
        ctx->BeginPath();
        ctx->MoveTo(10 * ratio, 24 * ratio);
        ctx->LineTo(100 * ratio, 200 * ratio);
        ctx->LineTo(50 * ratio, 34 * ratio);
        ctx->Fill();
    };
    testCases["tc_2d_fillRect"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->SetFillStyle("#ff0000");
        ctx->FillRect(0, 0, 100 * ratio, 100 * ratio);
    };
    testCases["tc_2d_bezierCurveTo"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->BeginPath();
        ctx->MoveTo(20 * ratio, 20 * ratio);
        ctx->BezierCurveTo(20 * ratio, 100 * ratio, 200 * ratio, 100 * ratio, 200 * ratio, 20 * ratio);
        ctx->Stroke();
    };
    testCases["tc_2d_text_fillText"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->SetFillStyle("#FF0000 ");
        ctx->SetFont("30px Arial");
        ctx->DrawText("w3school.com.cn", 10 * ratio, 50 * ratio);
    };
    //     testCases["tc_2d_line_dash"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
    //         int ratio = 1;
    //   ctx->SetFillStyle( = "#ff0000";

    //     var dashes = [10, 5];
    //     ctx->setLineDash(dashes);
    //     ctx->SetLineWidth( = 5*ratio;
    //     ctx->SetLineCap( = "butt";
    //     ctx->BeginPath();
    //     ctx->MoveTo(30*ratio, 100*ratio);
    //     ctx->LineTo(500*ratio, 100*ratio);
    //     ctx->Stroke();

    //     var dashes2 = ctx->getLineDash();
    //     dashes2.push(30);
    //     ctx->setLineDash(dashes2);

    //     ctx->BeginPath();
    //     ctx->MoveTo(30*ratio, 200*ratio);
    //     ctx->LineTo(500*ratio, 200*ratio);
    //     ctx->Stroke();
    //     };
    testCases["tc_2d_lineGap_butt"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->BeginPath();
        ctx->SetLineWidth(10 * ratio);
        ctx->SetLineCap("butt ");
        ctx->MoveTo(10 * ratio, 10 * ratio);
        ctx->LineTo(100 * ratio, 10 * ratio);
        ctx->Stroke();
    };
    testCases["tc_2d_lineGap_round"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->BeginPath();
        ctx->SetLineWidth(10 * ratio);
        ctx->SetLineCap("round");
        ctx->MoveTo(10 * ratio, 50 * ratio);
        ctx->LineTo(100 * ratio, 50 * ratio);
        ctx->Stroke();
    };
    testCases["tc_2d_lineGap_square"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->BeginPath();
        ctx->SetLineWidth(10 * ratio);
        ctx->SetLineCap("square ");
        ctx->MoveTo(10 * ratio, 100 * ratio);
        ctx->LineTo(100 * ratio, 100 * ratio);
        ctx->Stroke();
    };
    testCases["tc_2d_lineJoin_bevel"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->BeginPath();
        ctx->SetLineWidth(10 * ratio);
        ctx->SetLineJoin("bevel ");
        ctx->SetMiterLimit(5);
        ctx->MoveTo(10 * ratio, 10 * ratio);
        ctx->LineTo(200 * ratio, 100 * ratio);
        ctx->LineTo(50 * ratio, 200 * ratio);
        ctx->Stroke();
    };
    testCases["tc_2d_lineJoin_miter"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->BeginPath();
        ctx->SetLineWidth(10 * ratio);
        ctx->SetLineJoin("miter ");
        ctx->SetMiterLimit(5 * ratio);
        ctx->MoveTo(10 * ratio, 10 * ratio);
        ctx->LineTo(200 * ratio, 100 * ratio);
        ctx->LineTo(50 * ratio, 200 * ratio);
        ctx->Stroke();
    };
    testCases["tc_2d_lineJoin_round"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->BeginPath();
        ctx->SetLineWidth(10 * ratio);
        ctx->SetLineJoin("round ");
        ctx->SetMiterLimit(5 * ratio);
        ctx->MoveTo(10 * ratio, 10 * ratio);
        ctx->LineTo(200 * ratio, 100 * ratio);
        ctx->LineTo(50 * ratio, 200 * ratio);
        ctx->Stroke();
    };
    testCases["tc_2d_lineTo"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->BeginPath();
        ctx->MoveTo(0, 0);
        ctx->LineTo(150 * ratio, 150 * ratio);
        ctx->Stroke();
    };
    testCases["tc_2d_MDN_lineTo"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->MoveTo(90 * ratio, 130 * ratio);
        ctx->LineTo(95 * ratio, 25 * ratio);
        ctx->LineTo(150 * ratio, 80 * ratio);
        ctx->LineTo(205 * ratio, 25 * ratio);
        ctx->LineTo(210 * ratio, 130 * ratio);
        ctx->SetLineWidth(15 * ratio);
        ctx->Stroke();
    };
    testCases["tc_2d_quadraticCurveTo"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->BeginPath();
        ctx->MoveTo(10 * ratio, 10 * ratio);
        ctx->QuadraticCurveTo(20 * ratio, 100 * ratio, 200 * ratio, 50 * ratio);
        ctx->Stroke();
    };
    testCases["tc_2d_beginPath"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        // First path
        ctx->BeginPath();
        ctx->SetStrokeStyle("blue ");
        ctx->MoveTo(20 * ratio, 20 * ratio);
        ctx->LineTo(200 * ratio, 20 * ratio);
        ctx->Stroke();

        // Second path
        ctx->BeginPath();
        ctx->SetStrokeStyle("green ");
        ctx->MoveTo(20 * ratio, 20 * ratio);
        ctx->LineTo(120 * ratio, 120 * ratio);
        ctx->Stroke();
    };
    testCases["tc_2d_closePath"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;

        ctx->SetFillStyle("#000000 ");
        ctx->FillRect(0, 0, width, height);

        ctx->BeginPath();
        ctx->SetStrokeStyle("yellow");
        ctx->MoveTo(10 * ratio, 10 * ratio);
        ctx->LineTo(100 * ratio, 100 * ratio);
        ctx->LineTo(70 * ratio, 200 * ratio);
        ctx->ClosePath();
        ctx->Stroke();
    };
    testCases["tc_2d_lineTo"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->MoveTo(90 * ratio, 130 * ratio);
        ctx->LineTo(95 * ratio, 25 * ratio);
        ctx->LineTo(150 * ratio, 80 * ratio);
        ctx->LineTo(205 * ratio, 25 * ratio);
        ctx->LineTo(210 * ratio, 130 * ratio);
        ctx->SetLineWidth(15 * ratio);
        ctx->Stroke();
    };
    testCases["tc_2d_moveTo"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->BeginPath();
        ctx->MoveTo(50 * ratio, 50 * ratio);
        ctx->LineTo(50 * ratio, 150 * ratio);
        ctx->LineTo(150 * ratio, 150 * ratio);
        ctx->Stroke();
    };
    testCases["tc_2d_shadow"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->SetFillStyle("blue");
        ctx->SetShadowColor("black");
        ctx->SetShadowBlur(20 * ratio);
        ctx->SetShadowOffsetX(20 * ratio);
        ctx->SetShadowOffsetY(20 * ratio);
        ctx->FillRect(10 * ratio, 10 * ratio, 100 * ratio, 100 * ratio);
    };
    testCases["tc_2d_shadowOffsetX"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        // Shadow
        ctx->SetShadowColor("red ");
        ctx->SetShadowOffsetX(25 * ratio);
        ctx->SetShadowBlur(10 * ratio);

        // Rectangle
        ctx->SetFillStyle("blue ");
        ctx->FillRect(20 * ratio, 20 * ratio, 150 * ratio, 100 * ratio);
    };
    testCases["tc_2d_shadowOffsetY"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        // Shadow
        ctx->SetShadowColor("red ");
        ctx->SetShadowOffsetY(25 * ratio);
        ctx->SetShadowBlur(10 * ratio);

        // Rectangle
        ctx->SetFillStyle("blue ");
        ctx->FillRect(20 * ratio, 20 * ratio, 150 * ratio, 80 * ratio);
    };
    testCases["tc_2d_shaow_blur"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->SetFillStyle("blue");
        ctx->SetShadowBlur(20 * ratio);
        ctx->SetShadowColor("black");
        ctx->SetShadowOffsetX(10 * ratio);
        ctx->SetShadowOffsetY(10 * ratio);
        ctx->FillRect(40 * ratio, 100 * ratio, 200 * ratio, 200 * ratio);
    };
    testCases["tc_2d_stroke_alpha"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->SetStrokeStyle("rgba(255,0,0,0.5) ");
        ctx->SetLineWidth(5 * ratio);
        ctx->StrokeRect(10 * ratio, 10 * ratio, 100 * ratio, 100 * ratio);
    };
    testCases["tc_2d_stroke"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;

        ctx->SetFillStyle("#000000 ");
        ctx->FillRect(0, 0, width, height);

        ctx->SetStrokeStyle("rgba(255,0,0,1.0) ");
        ctx->SetLineWidth(5 * ratio);
        ctx->Rect(10 * ratio, 10 * ratio, 100 * ratio, 100 * ratio);
        ctx->Stroke();
    };
    testCases["tc_2d_strokeRect"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->SetStrokeStyle("rgba(255,0,255,1.0)");
        ctx->SetLineWidth(5 * ratio);
        ctx->StrokeRect(10 * ratio, 10 * ratio, 100 * ratio, 100 * ratio);
    };
    //     testCases["tc_2d_strokeStyle"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
    //         int ratio = 1;
    //         for (let i = 0; i < 6; i++)
    //         {
    //             for (let j = 0; j < 6; j++)
    //             {
    //     ctx->SetStrokeStyle( = `rgb(
    //         0,
    //         ${Math.floor(255 - 42.5 * i)},
    //         ${Math.floor(255 - 42.5 * j)})`;
    //     ctx->BeginPath();
    //     ctx->Arc((12.5 + j * 25) *ratio, (12.5 + i * 25) *ratio, 10 *ratio, 0, M_PI * 2, true);
    //     ctx->Stroke();
    //             }
    //         }
    //     };
    testCases["tc_2d_MDN_scale"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        // Scaled rectangle
        ctx->Scale(9, 3);
        ctx->SetFillStyle("red ");
        ctx->FillRect(10 * ratio, 10 * ratio, 8 * ratio, 20 * ratio);

        // Reset current transformation matrix to the identity matrix
        ctx->SetTransform(1, 0, 0, 1, 0, 0);

        // Non-Scaled rectangle
        ctx->SetFillStyle("gray ");
        ctx->FillRect(10 * ratio, 10 * ratio, 8 * ratio, 20 * ratio);
    };
    testCases["tc_2d_MDN_transform"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->Transfrom(1, .2, .8, 1, 0, 0);
        ctx->FillRect(0, 0, 100 * ratio, 100 * ratio);
    };

    // testCases["tc_2d_get_put_ImageData"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
    //     int ratio = 1;
    //     ctx->SetFillStyle("#000000");
    //     ctx->rect(10 * ratio, 10 * ratio, 100 * ratio, 100 * ratio);
    //     ctx->Fill();

    //     let imageData = ctx->getImageData(60 * ratio, 60 * ratio, 200 * ratio, 100 * ratio);
    //     ctx->putImageData(imageData, 150 * ratio, 10 * ratio);

    // ctx->SetFillStyle( = "red";
    // ctx->FillRect(50*ratio, 50*ratio, 50*ratio, 50*ratio);

    // let imgData = ctx->getImageData(50*ratio, 50*ratio, 50*ratio, 50*ratio);
    // ctx->putImageData(imgData, 50*ratio, 150*ratio);
    // };

    testCases["tc_2d_rotate"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->Rotate(20 * M_PI / 180);
        ctx->SetFillStyle("yellow");
        ctx->FillRect(50 * ratio, 20 * ratio, 100 * ratio, 50 * ratio);
    };
    testCases["tc_2d_save"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->SetFillStyle("#000000 ");
        ctx->Save();
        ctx->SetFillStyle("red ");
        ctx->FillRect(10 * ratio, 10 * ratio, 150 * ratio, 100 * ratio);
        ctx->Restore();
        ctx->FillRect(50 * ratio, 50 * ratio, 150 * ratio, 100 * ratio);
    };
    testCases["tc_2d_scale"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->StrokeRect(10 * ratio, 10 * ratio, 100 * ratio, 50 * ratio);
        ctx->Scale(2, 2);
        ctx->StrokeRect(10 * ratio, 10 * ratio, 100 * ratio, 50 * ratio);
    };
    testCases["tc_2d_resetTransform"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->SetFillStyle("#000000");
        ctx->Rotate(45 * M_PI / 180);
        ctx->FillRect(70 * ratio, 0, 100 * ratio, 30 * ratio);
        ctx->ResetTransform();
        ctx->SetFillStyle("red");
        ctx->FillRect(70 * ratio, 0, 100 * ratio, 30 * ratio);
    };
    testCases["tc_2d_transform"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->SetFillStyle("yellow");
        ctx->FillRect(10 * ratio, 10 * ratio, 100 * ratio, 50 * ratio);
        ctx->Transfrom(1, 0.5, -0.5, 1, 30 * ratio, 10 * ratio);
        ctx->SetFillStyle("red");
        ctx->FillRect(10 * ratio, 10 * ratio, 100 * ratio, 50 * ratio);
        ctx->Transfrom(1, 0.5, -0.5, 1, 30 * ratio, 10 * ratio);
        ctx->SetFillStyle("blue");
        ctx->FillRect(10 * ratio, 10 * ratio, 100 * ratio, 50 * ratio);
    };
    testCases["tc_2d_translate"] = [](std::shared_ptr<gcanvas::GCanvas> canvas, GCanvasContext *ctx, int width, int height) {
        int ratio = 1;
        ctx->SetFillStyle("#000000");
        ctx->FillRect(10 * ratio, 10 * ratio, 100 * ratio, 50 * ratio);
        ctx->Translate(100 * ratio, 100 * ratio);
        ctx->FillRect(10 * ratio, 10 * ratio, 100 * ratio, 50 * ratio);
    };
}