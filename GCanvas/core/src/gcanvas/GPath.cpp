/**
 * Created by G-Canvas Open Source Team.
 * Copyright (c) 2017, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information, please view
 * the LICENSE file in the root directory of this source tree.
 */
#include "GPath.h"
#include "GCanvas2dContext.h"

#define  G_PATH_RECURSION_LIMIT 8
#define  G_PATH_DISTANCE_EPSILON 1.0f
#define  G_PATH_COLLINEARITY_EPSILON FLT_EPSILON
#define  G_PATH_STEPS_FOR_CIRCLE 48.0f
#define  G_PATH_ANGLE_EPSILON = 0.01;

#define  ERROR_DEVIATION    1e-6
#define  PI_1               M_PI
#define  PI_2               2.f * M_PI
#define  DEFAULT_STEP_COUNT 100

static bool isSamePoint(GPoint& p1, GPoint& p2, float min)
{
    return abs(p1.x - p2.x) < min && abs(p1.y - p2.y) < min;
}

static bool isNan(const GPoint& p)
{
    return std::isnan(p.x) || std::isnan(p.y);
}


static bool hasSamePoint(GPoint& p1, GPoint& p2, GPoint& p3, float minValue)
{
    return isSamePoint(p1, p2, minValue) || isSamePoint(p2, p3, minValue) || isSamePoint(p1, p3, minValue);
}


static bool isTrianglePointsValid(GPoint& p1, GPoint& p2, GPoint& p3, float minValue)
{
    if (isNan(p1) || isNan(p3) || isNan(p2))
    {
        return false;
    }
    return hasSamePoint(p1, p2, p3, minValue) ? false : true;
}


GPath::GPath() { Reset(); }

GPath::GPath(const GPath &other) {
    mDistanceTolerance = other.mDistanceTolerance;
    mHasInitStartPosition = other.mHasInitStartPosition;
    mStartPosition = other.mStartPosition;
    mCurrentPosition = other.mCurrentPosition;
    mCurPath = other.mCurPath;
    mPathStack = other.mPathStack;
    mFillRule = other.mFillRule;
    mMinPosition = other.mMinPosition;
    mMaxPosition = other.mMaxPosition;
}

tSubPath &GPath::GetCurPath() {
    if (mPathStack.empty()) {
        mPathStack.resize(1);
    }

    return mPathStack[mPathStack.size() - 1];
}

void GPath::Reset() {
    mPathStack.clear();
    tSubPath &curPath = GetCurPath();
    curPath.isClosed = false;
    curPath.points.clear();
    mHasInitStartPosition = false;

    mStartPosition = PointMake(0, 0);
    mCurrentPosition = PointMake(0, 0);
    
    mFillRule = FILL_RULE_NONZERO;
    mMinPosition = PointMake(INFINITY, INFINITY);
    mMaxPosition = PointMake(-INFINITY, -INFINITY);
    
}

void GPath::EndSubPath() {
    tSubPath &curPath = GetCurPath();
    if (!curPath.points.empty()) mPathStack.resize(mPathStack.size() + 1);

    GetCurPath().isClosed = false;
    mStartPosition = mCurrentPosition;
    mHasInitStartPosition = true;
}

void GPath::push(GPoint pt) { push(pt.x, pt.y); }

void GPath::push(float x, float y) {
    GPoint p = PointMake(x, y);
    
    tSubPath &curPath = GetCurPath();
    
    curPath.points.push_back(p);
    mCurrentPosition.x = x;
    mCurrentPosition.y = y;
    mMinPosition.x = std::min<float>( mMinPosition.x, x );
    mMinPosition.y = std::min<float>( mMinPosition.y, y );
    mMaxPosition.x = std::max<float>( mMaxPosition.x, x );
    mMaxPosition.y = std::max<float>( mMaxPosition.y, y );
}


void GPath::MoveTo(float x, float y) {
    EndSubPath();

    mStartPosition = PointMake(x, y);
    mHasInitStartPosition = true;
    push(x, y);
}

void GPath::LineTo(float x, float y) { push(x, y); }

void GPath::Close() {
    GetCurPath().isClosed = true;
    if (mHasInitStartPosition) {
        push(mStartPosition);
    }
    EndSubPath();
}

void GPath::QuadraticCurveTo(float cpx, float cpy, float x, float y,
                             float scale) {
    mDistanceTolerance = G_PATH_DISTANCE_EPSILON / scale;
    mDistanceTolerance *= mDistanceTolerance;

    recursiveQuadratic(mCurrentPosition.x, mCurrentPosition.y, cpx, cpy, x, y, 0);

    push(x, y);
}

void GPath::recursiveQuadratic(float x1, float y1, float x2, float y2,
                               float x3, float y3, int level) {
    float x12 = (x1 + x2) / 2;
    float y12 = (y1 + y2) / 2;
    float x23 = (x2 + x3) / 2;
    float y23 = (y2 + y3) / 2;
    float x123 = (x12 + x23) / 2;
    float y123 = (y12 + y23) / 2;

    float dx = x3 - x1;
    float dy = y3 - y1;
    float d = fabsf(((x2 - x3) * dy - (y2 - y3) * dx));

    if (d > G_PATH_COLLINEARITY_EPSILON) {
        // Regular care
        if (d * d <= mDistanceTolerance * (dx * dx + dy * dy)) {
            push(x123, y123);
            return;
        }
    } else {
        // Collinear case
        dx = x123 - (x1 + x3) / 2;
        dy = y123 - (y1 + y3) / 2;
        if (dx * dx + dy * dy <= mDistanceTolerance) {
            push(x123, y123);
            return;
        }
    }

    if (level <= G_PATH_RECURSION_LIMIT) {
        // Continue subdivision
        recursiveQuadratic(x1, y1, x12, y12, x123, y123, level + 1);
        recursiveQuadratic(x123, y123, x23, y23, x3, y3, level + 1);
    }
}


void GPath::BezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y,
                          float x, float y, float scale) {
    mDistanceTolerance = G_PATH_DISTANCE_EPSILON / scale;
    mDistanceTolerance *= mDistanceTolerance;

    GPoint points[4] = {
            PointMake(mCurrentPosition.x, mCurrentPosition.y),
            PointMake(cp1x, cp1y),
            PointMake(cp2x, cp2y),
            PointMake(x, y),
    };

    SubdivideCubicTo(this, points, 4);
    push(x, y);
}

void GPath::recursiveBezier(float x1, float y1, float x2, float y2, float x3,
                float y3, float x4, float y4, int level) {
    // Based on http://www.antigrain.com/research/adaptive_bezier/index.html

//     Calculate all the mid-points of the line segments
    float x12 = (x1 + x2) / 2;
    float y12 = (y1 + y2) / 2;
    float x23 = (x2 + x3) / 2;
    float y23 = (y2 + y3) / 2;
    float x34 = (x3 + x4) / 2;
    float y34 = (y3 + y4) / 2;
    float x123 = (x12 + x23) / 2;
    float y123 = (y12 + y23) / 2;
    float x234 = (x23 + x34) / 2;
    float y234 = (y23 + y34) / 2;
    float x1234 = (x123 + x234) / 2;
    float y1234 = (y123 + y234) / 2;

    if (level > 0) {
        float dx = x4 - x1;
        float dy = y4 - y1;

        float d2 = fabsf(((x2 - x4) * dy - (y2 - y4) * dx));
        float d3 = fabsf(((x3 - x4) * dy - (y3 - y4) * dx));

        if (d2 > G_PATH_COLLINEARITY_EPSILON &&
            d3 > G_PATH_COLLINEARITY_EPSILON) {
            // Regular care
            if ((d2 + d3) * (d2 + d3) <=
                mDistanceTolerance * (dx * dx + dy * dy)) {
                push(x1234, y1234);
                return;
            }
        } else {
            if (d2 > G_PATH_COLLINEARITY_EPSILON) {
                // p1,p3,p4 are collinear, p2 is considerable
                if (d2 * d2 <= mDistanceTolerance * (dx * dx + dy * dy)) {
                    push(x1234, y1234);
                    return;
                }
            } else if (d3 > G_PATH_COLLINEARITY_EPSILON) {
                // p1,p2,p4 are collinear, p3 is considerable
                if (d3 * d3 <= mDistanceTolerance * (dx * dx + dy * dy)) {
                    push(x1234, y1234);
                    return;
                }
            } else {
                // Collinear case
                dx = x1234 - (x1 + x4) / 2;
                dy = y1234 - (y1 + y4) / 2;
                if (dx * dx + dy * dy <= mDistanceTolerance) {
                    push(x1234, y1234);
                    return;
                }
            }
        }
    }

    if (level <= G_PATH_RECURSION_LIMIT) {
        // Continue subdivision
        recursiveBezier(x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1);
        recursiveBezier(x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1);
    }
}


void GPath::ArcTo(float x1, float y1, float x2, float y2, float radius) {
    // Lifted from http://code.google.com/p/fxcanvas/
    GPoint cp = mCurrentPosition;

    float a1 = cp.y - y1;
    float b1 = cp.x - x1;
    float a2 = y2 - y1;
    float b2 = x2 - x1;
    float mm = fabsf(a1 * b2 - b1 * a2);

    if (mm < 1.0e-8 || radius == 0) {
        LineTo(x1, y1);
    } else {
        float dd = a1 * a1 + b1 * b1;
        float cc = a2 * a2 + b2 * b2;
        float tt = a1 * a2 + b1 * b2;
        float k1 = radius * sqrtf(dd) / mm;
        float k2 = radius * sqrtf(cc) / mm;
        float j1 = k1 * tt / dd;
        float j2 = k2 * tt / cc;
        float cx = k1 * b2 + k2 * b1;
        float cy = k1 * a2 + k2 * a1;
        float px = b1 * (k2 + j1);
        float py = a1 * (k2 + j1);
        float qx = b2 * (k1 + j2);
        float qy = a2 * (k1 + j2);
        float startAngle = atan2f(py - cy, px - cx);
        float endAngle = atan2f(qy - cy, qx - cx);

        Arc(cx + x1, cy + y1, radius, startAngle, endAngle,
            (b1 * a2 > b2 * a1));
    }
}

void GPath::Arc(float cx, float cy, float radius, float startAngle,
                float endAngle, bool antiClockwise) {
    float spanAngle = endAngle - startAngle;
    if (antiClockwise) {
        spanAngle = -spanAngle;
    }

    float spanAngleAbs = fabs(spanAngle);
    if (spanAngleAbs < ERROR_DEVIATION) { // the same angle,do nothing
        return;
    }
    
    float pi2 = PI_2;

    if (spanAngleAbs > pi2) {
        spanAngle = pi2;
    } else {
        while (spanAngle < 0) {
            spanAngle += pi2 * 2;
        }
        while (spanAngle > pi2) {
            spanAngle -= pi2;
        }
    }

    int stepCount = DEFAULT_STEP_COUNT * (spanAngle / pi2);
    bool isCircle = (stepCount == DEFAULT_STEP_COUNT);
    
    
    if (stepCount < 1) {
        stepCount = 1;
    }
    float deltaAngle = spanAngle / stepCount;
    if (antiClockwise) {
        deltaAngle = -deltaAngle;
    }
    
    float dx = cosf(deltaAngle);
    float dy = sinf(deltaAngle);
    
    float xOffset = radius * cosf(startAngle);
    float yOffset = radius * sinf(startAngle);
    
    for (int step = 0; step <= stepCount; ++step) {
        if (step == 0) {
            
            if (isCircle) { //remove moveTo while isCircle
                if ( !GetCurPath().isClosed ){
                    tSubPath &curPath = GetCurPath();
                    if (curPath.points.size() == 1) {
                        curPath.points.pop_back();
                    }
                }
            }
            
            if ( !GetCurPath().isClosed ){
                tSubPath &curPath = GetCurPath();
                if (curPath.points.size() == 0) {
                    mHasInitStartPosition = true;
                    mStartPosition = PointMake(cx + xOffset, cy + yOffset);
                }
            }
        }
        push(cx + xOffset, cy + yOffset);
        
        float oldX = xOffset, oldY = yOffset;
        xOffset = oldX * dx - oldY * dy;
        yOffset = oldX * dy + oldY * dx;
    }
}



void GPath::ClipRegion(GCanvasContext *context)
{
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    glEnable(GL_STENCIL_TEST);
    // clip use high mask bit
    GLuint mask = 0x80;
    GLuint ref = 0x80;
    glStencilMask(mask);

    // use stencil func
    if (context->HasClipRegion()) {
        glStencilFunc(GL_EQUAL, ref, mask);
        glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
    } else {
        glClear(GL_STENCIL_BUFFER_BIT);
        glStencilFunc(GL_ALWAYS, ref, mask);
        glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
    }

    for (std::vector<tSubPath>::const_iterator pathIter =
            mPathStack.begin();
         pathIter != mPathStack.end(); ++pathIter) {
        const std::vector<GPoint> &path = pathIter->points;
        if (path.size() < 3) {
            continue;
        }

        glVertexAttribPointer(context->PositionSlot(), 2, GL_FLOAT, GL_FALSE,
                              sizeof(GPoint), &(path.front()));
        context->mDrawCallCount++;
        glDrawArrays(GL_TRIANGLE_FAN, 0, (GLsizei) path.size());
    }
    context->BindPositionVertexBuffer();
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // reset stencil
    SetStencilForClip();
}
void GPath::PushTriangleFanPoints(GCanvasContext *context, tSubPath* subPath, GColorRGBA color)
{
    std::vector<GPoint> &pts = subPath->points;
    if (subPath->isClosed)
    {
        context->PushTriangleFanPoints(pts, color);
    }
    else
    {
        // push head point
        pts.push_back(pts.front());
        context->PushTriangleFanPoints(pts, color);
        pts.pop_back();
    }
    context->SendVertexBufferToGPU(GL_TRIANGLE_FAN);
}

void GPath::DrawPolygons2DToContext(GCanvasContext *context, GFillRule rule, GFillTarget target )
{
    
    GLint bindFBO = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &bindFBO);
    
    
    context->SendVertexBufferToGPU();
    
    GColorRGBA color = BlendColor(context, context->mCurrentState->mFillColor);
    
    // Disable drawing to the color buffer, enable the stencil buffer
    if (context->mCurrentState->mShader->GetTexcoordSlot() > 0) {
        glDisableVertexAttribArray((GLuint)context->mCurrentState->mShader->GetTexcoordSlot());
    }
    glDisableVertexAttribArray((GLuint)context->mCurrentState->mShader->GetColorSlot());
    
    glDisable(GL_BLEND);
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xff);
    
    glStencilFunc(GL_ALWAYS, 0, 0xff);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    
    // Clear the needed area in the stencil buffer
    glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
    GColorRGBA white = {1,1,1,1};
    
    GPoint minPos = mMinPosition;
    GPoint maxPos = mMaxPosition;
    
    context->PushRectangle(minPos.x, minPos.y, maxPos.x-minPos.x, maxPos.y-minPos.y, 0, 0, 0, 0, white);
    context->SendVertexBufferToGPU();
    
    
    if( rule == FILL_RULE_NONZERO )
    {
        glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
        glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);
    }
    else if( rule == FILL_RULE_EVENODD )
    {
        glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
    }
    
    for (std::vector<tSubPath>::iterator iter = mPathStack.begin(); iter != mPathStack.end(); ++iter)
    {
        tSubPath path = *iter;
        
        if( path.points.size() == 0 ) continue;
        
        glVertexAttribPointer((GLuint) context->mCurrentState->mShader->GetPositionSlot(), 2,
                              GL_FLOAT, GL_FALSE, 0,
                              &(path.points.front()));
        context->mDrawCallCount++;
        glDrawArrays(GL_TRIANGLE_FAN, 0, (int)path.points.size());
    }
    
    context->BindVertexBuffer();
    
    //enable color or depth buffer, push a rect with the correct size and color
    if( target == FILL_TARGET_DEPTH ) {
        glDepthFunc(GL_ALWAYS);
        glDepthMask(GL_TRUE);
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    else if( target == FILL_TARGET_COLOR ) {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glEnable(GL_BLEND);
    }
    
    glStencilFunc(GL_NOTEQUAL, 0x00, 0xff);
    glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
    
    context->PushRectangle(minPos.x, minPos.y, maxPos.x-minPos.x, maxPos.y-minPos.y, 0, 0, 0, 0, color);
    context->SendVertexBufferToGPU();
    
    glDisable(GL_STENCIL_TEST);
    
    if( target == FILL_TARGET_DEPTH )
    {
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_EQUAL);
        
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glEnable(GL_BLEND);
    }
}

void GPath::drawArcToContext(GCanvasContext *context, GPoint point,
                             GPoint p1, GPoint p2, GColorRGBA color, std::vector<GVertex> *vec, float samePointThreshold) {
    // filter invalid point
    bool fixAndroidCompatible = false;
#ifdef ANDROID
    fixAndroidCompatible = true;
#endif

    float minValue = samePointThreshold;
    // LOG_I("drawArcToContext p1(%f,%f),p2(%f,%f),p3(%f,%f)", p1.x, p1.y, p2.x, p2.y, point.x,point.y);
    if (fixAndroidCompatible && (isSamePoint(point, p1, minValue) || isSamePoint(p1, p2, minValue) || isSamePoint(point, p2, minValue)))
    {
        // LOG_I("isSamePoint, exit: ");
        return;
    }

    // filter nan pint
    if (isNan(point) || isNan(p1) || isNan(p2))
    {
        // LOG_I("isNan check true");
        return;
    }

    float width2 = context->LineWidth();
    if( width2 >= 2 ){
        width2 /= 2.f;
    }

    GPoint v1 = PointNormalize(PointSub(p1, point)),
            v2 = PointNormalize(PointSub(p2, point));

    float angle2;
    if (v1.x == -v2.x && v1.y == -v2.y) {
        angle2 = M_PI;
    } else {
        angle2 = acosf(v1.x * v2.x + v1.y * v2.y);
    }

    if (fixAndroidCompatible && std::isnan(angle2)) {
        return;
    }

    // 1 step per 5 pixel
    float needStep =  (angle2 * width2) / 5.0f;
    int numSteps = 0;
    if (fixAndroidCompatible)
    {
        if (needStep < minValue)
        {
            numSteps = 0;
        }
        else if (needStep < 1)
        {
            numSteps = 1;
        }
        else
        {
            numSteps = (int)std::min<float>(64, std::max<float>(20, needStep));
        }
    }
    else
    {
        numSteps = (int)std::min<float>(64, std::max<float>(20, needStep));
    }

    // LOG_E("needStep=%f, steps=%i", needStep, numSteps);
    if (numSteps > 0)
    {
        // calculate angle step
        float step = (angle2 / numSteps);
        float angle1 = atan2(1.0f, 0.0f) - atan2(v1.x, -v1.y);
        // starting point
        float angle = angle1;

        GPoint arcP1 = {point.x + cosf(angle) * width2,
                        point.y - sinf(angle) * width2};
        GPoint arcP2;
        for (int i = 0; i < numSteps; i++) {
            angle += step;
            arcP2 = PointMake(point.x + cosf(angle) * width2,
                              point.y - sinf(angle) * width2);

            context->PushTriangle(arcP1, point, arcP2, color, vec);
            arcP1 = arcP2;
        }
    }
}

std::vector<tSubPath>* GPath::DrawLineDash(GCanvasContext *context)
{
    std::vector<float> lineDash = context->LineDash();
    std::vector<float> firstLineDash = context->LineDash();
    bool firstNeedDraw = true;
    int firstDashIndex = 0;
    float lineDashOffset = context->LineDashOffset();
    // deal offset
    if (lineDashOffset > 0.1 && lineDashOffset < 50) {
        float totalLength = 0;
        for (int i = 0; i < lineDash.size(); i++) {
            totalLength += lineDash[i];
        }
        while (lineDashOffset > totalLength) {
            lineDashOffset -= totalLength;
            if (lineDash.size() % 2 == 1) {
                firstNeedDraw = !firstNeedDraw;
            }
        }

        for (int i = 0; i < firstLineDash.size(); i++) {
            float dash = firstLineDash[i];
            if (dash > lineDashOffset) {
                firstLineDash[i] = dash - lineDashOffset;
                break;
            }
            else {
                firstNeedDraw = !firstNeedDraw;
                lineDashOffset -= dash;
                firstLineDash[i] = 0;
                firstDashIndex++;
                if (i == firstLineDash.size() - 1) {
                    // reset
                    firstDashIndex = 0;
                    firstLineDash = lineDash;
                }
            }
        }
    }

    std::vector<tSubPath> *tmpPathStack = new std::vector<tSubPath>;
    for (std::vector<tSubPath>::const_iterator iter = mPathStack.begin();
         iter != mPathStack.end(); ++iter) {
        const std::vector<GPoint> &pts = iter->points;
        GPoint startPoint, stopPoint;
        if (pts.size() <= 1) {
            continue;
        }

        // dashwidth
        int dashIndex = firstDashIndex;
        float dashWidth = firstLineDash[dashIndex];
        bool needDraw = firstNeedDraw;
        bool isFirst = true;

        // new tmp path
        tSubPath tmpPath;
        tmpPath.isClosed = false;
        float currentWidth = 0;
        stopPoint = *(pts.begin());
        tmpPath.points.push_back(stopPoint);

        for (std::vector<GPoint>::const_iterator ptIter = pts.begin() + 1;
             ptIter != pts.end(); ) {
            startPoint = stopPoint;
            stopPoint = *ptIter;
            // calc cur length
            float deltaX = stopPoint.x - startPoint.x;
            float deltaY = stopPoint.y - startPoint.y;
            float length = sqrtf(deltaX * deltaX + deltaY * deltaY);

            if (length + currentWidth > dashWidth) {
                // insert point
                float gapWidth = dashWidth - currentWidth;
                float interDeltaX = deltaX * gapWidth / length;
                float interDeltaY = deltaY * gapWidth / length;
                GPoint interPoint = PointMake(interDeltaX + startPoint.x, interDeltaY + startPoint.y);
                tmpPath.points.push_back(interPoint);

                if (needDraw) {
                    tmpPathStack->push_back(tmpPath);
                }

                // new path, update variables
                tmpPath.points.clear();
                tmpPath.isClosed = false;
                needDraw = !needDraw;
                currentWidth = 0;
                tmpPath.points.push_back(interPoint);
                stopPoint = interPoint;

                // update dash
                if (isFirst) {
                    dashIndex++;
                    if (dashIndex >= firstLineDash.size()) {
                        isFirst = false;
                        dashIndex = 0;
                        dashWidth = lineDash[dashIndex];
                    }
                    else {
                        dashWidth = firstLineDash[dashIndex];
                    }
                }
                else {
                    dashIndex = (dashIndex + 1) % lineDash.size();
                    dashWidth = lineDash[dashIndex];
                }

                continue;

            }
            else {
                tmpPath.points.push_back(stopPoint);
                currentWidth += length;
                if (ptIter == pts.end() - 1) {
                    // finished，push vertex
                    if (needDraw) {
                        tmpPathStack->push_back(tmpPath);
                    }
                }
            }
            ++ptIter;
        }
    }
    return tmpPathStack;
}


void GPath::CreateLinesFromPoints(GCanvasContext *context, GColorRGBA color, std::vector<GVertex> *vertexVec)
{
    float lineWidth = context->LineWidth() * 0.5;
    float minValidValue = 0.01;

    std::vector<tSubPath> *pathStack = &mPathStack;
    // first deal line dash if needed
    if (context->LineDash().size() > 0) {
        pathStack = DrawLineDash(context);
    }

    for (std::vector<tSubPath>::iterator iter = pathStack->begin();
         iter != pathStack->end(); ++iter) {
        std::vector<GPoint> &pts = iter->points;
        bool subPathIsClosed = iter->isClosed;
        bool firstInSub = true;
        GPoint startPoint, stopPoint;
        GPoint secondPoint = {0, 0};
        GPoint miter11, miter12; // Current miter vertices (left, right)
        GPoint miter21, miter22; // Next miter vertices (left, right)
        if (pts.size() <= 1) {
            continue;
        }

        stopPoint = *(pts.begin());
        // LOG_E("pathStack pointSize=%f,%f %f,%f %f,%f", pts[0].x, pts[0].y,pts[1].x, pts[1].y,pts[2].x, pts[2].y);
        
        // filter close point
        GPoint prePoint = *(pts.begin());
        for (std::vector<GPoint>::const_iterator ptIter = pts.begin() + 1;
             ptIter != pts.end(); ) {
            float deltaX = (*ptIter).x - prePoint.x;
            float deltaY = (*ptIter).y - prePoint.y;
            float length = sqrtf(deltaX * deltaX + deltaY * deltaY);
            if (length < minValidValue) {
                ptIter = pts.erase(ptIter);
            }
            else {
                prePoint = *ptIter;
                ptIter++;
            }
        }
        
        for (std::vector<GPoint>::const_iterator ptIter = pts.begin() + 1;
             ptIter != pts.end(); ++ptIter) {

            // LOG_E("pathStack iterate point=%f,%f", ptIter->x, ptIter->y);
            float deltaX = (*ptIter).x - stopPoint.x;
            float deltaY = (*ptIter).y - stopPoint.y;

            float length = sqrtf(deltaX * deltaX + deltaY * deltaY);
            
            std::vector<GPoint>::const_iterator nextIter = ptIter + 1;
            
            startPoint = stopPoint;
            stopPoint = *ptIter;

            float temp = deltaX;
            deltaX = deltaY * lineWidth / length;
            deltaY = temp * lineWidth / length;

            miter11 = PointMake(startPoint.x - deltaX, startPoint.y + deltaY);
            miter12 = PointMake(startPoint.x + deltaX, startPoint.y - deltaY);
            miter21 = PointMake(stopPoint.x - deltaX, stopPoint.y + deltaY);
            miter22 = PointMake(stopPoint.x + deltaX, stopPoint.y - deltaY);

            // step1: draw line
            context->PushQuad(miter11, miter21, miter22, miter12, color, vertexVec);

            // if lineWidth > 1, draw mitter
            if (context->LineWidth() <= 1) {
                continue;
            }

            // step2: draw head cap
            if (firstInSub) {
                firstInSub = false;
                secondPoint = stopPoint;
                if (!subPathIsClosed)
                {
                    drawLineCap(context, startPoint, miter12, miter11, -deltaY,
                                -deltaX, color, vertexVec);
                }
            }
            // step3: draw tail cap
            if (nextIter == pts.end() && (!subPathIsClosed)) {
                drawLineCap(context, stopPoint, miter21, miter22, deltaY,
                            deltaX, color, vertexVec);
                break;
            }

            // step4: draw line join
            GPoint nextCenter;
            if (nextIter != pts.end()) {
                nextCenter = *nextIter;
            } else {
                nextCenter = secondPoint;
            }

            float nx = nextCenter.x - stopPoint.x;
            float ny = nextCenter.y - stopPoint.y;
            float nl = sqrtf(nx * nx + ny * ny);
            float nt = nx;
            nx = ny * lineWidth / nl;
            ny = nt * lineWidth / nl;

            GPoint n11, n12;
            n11 = PointMake(stopPoint.x - nx, stopPoint.y + ny);
            n12 = PointMake(stopPoint.x + nx, stopPoint.y - ny);

            float nextAngle = calcPointAngle(nextCenter, stopPoint);
            float curAngle = calcPointAngle(startPoint, stopPoint);
            float angleDiff = nextAngle - curAngle;
            if (angleDiff < 0) {
                angleDiff += PI_1 * 2;
            }

            GPoint joinP1, joinP2;
            if (angleDiff > PI_1) {
                joinP1 = n12;
                joinP2 = miter22;
            } else {
                joinP1 = miter21;
                joinP2 = n11;
            }


            if (context->LineJoin() == LINE_JOIN_ROUND) {
                drawArcToContext(context, stopPoint, joinP1, joinP2, color, vertexVec, minValidValue);
            } else if (context->LineJoin() == LINE_JOIN_MITER) {
                drawLineJoinMiter(context, stopPoint, joinP2, joinP1, color, vertexVec);
            } else if (context->LineJoin() == LINE_JOIN_BEVEL) {
                if (isTrianglePointsValid(stopPoint, joinP1, joinP2, minValidValue))
                {
                    context->PushTriangle(stopPoint, joinP1, joinP2, color, vertexVec);
                }
                else
                {
                    //LOG_E("invalid bevel join pointSize=%f,%f %f,%f %f,%f",
                    //      stopPoint.x, stopPoint.y,joinP1.x, joinP1.y,joinP2.x, joinP2.y);
                }
            }
        }
    }

    // delete pathStack
    if (context->LineDash().size() > 0) {
        delete pathStack;
    }
}

void GPath::DrawLinesToContext(GCanvasContext *context)
{
    context->SetTexture(InvalidateTextureId);
    GColorRGBA color = BlendStrokeColor(context);
    
    std::vector<GVertex> vertexVec;

    if (color.rgba.a < 1.0) { //transparent, use stencil buffer
        CreateLinesFromPoints(context, color, &vertexVec);
    }
    else {
        CreateLinesFromPoints(context, color, NULL);
        return;
    }

    StencilRectForStroke(context, vertexVec);
}

void GPath::StencilRectForStroke(GCanvasContext *context, std::vector<GVertex> &vertexVec)
{
    context->SendVertexBufferToGPU();
    GColorRGBA color = BlendStrokeColor(context);
    
    //set environment
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glEnable(GL_STENCIL_TEST);
    
    // clear stencil buffer
    glStencilFunc(GL_ALWAYS, 0, 0xff);
    glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
    GColorRGBA white = {1,1,1,1};
    
    float extend = context->MiterLimit() * context->LineWidth();
    if (extend < context->LineWidth() * 0.5) {
        extend = context->LineWidth() * 0.5;
    }
    GRect rect;
    rect.x = mMinPosition.x - extend;
    rect.y = mMinPosition.y - extend;
    rect.width = mMaxPosition.x - mMinPosition.x + extend * 2;
    rect.height = mMaxPosition.y - mMinPosition.y + extend * 2;
    if (rect.height > context->GetHeight() || rect.width > context->GetWidth()) {
        rect.x = 0;
        rect.y = 0;
        rect.width = context->GetWidth();
        rect.height = context->GetHeight();
    }
    
    context->PushRectangle(rect.x, rect.y, rect.width, rect.height, 0, 0, 0, 0, white);
    context->SendVertexBufferToGPU();
    
    // first draw stencil
    glStencilFunc(GL_ALWAYS, 0x1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    
    // push vertexs
    context->PushVertexs(vertexVec);
    context->SendVertexBufferToGPU();
    
    // second draw color buffer with stencil buffer
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilFunc(GL_NOTEQUAL, 0x00, 0xff);
    glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
    
    context->PushRectangle(rect.x, rect.y, rect.width, rect.height, 0, 0, 0, 0, color);
    context->SendVertexBufferToGPU();
    
    // reset
    glDisable(GL_STENCIL_TEST);
}

float GPath::calcPointAngle(const GPoint &director, const GPoint &center) {
    GPoint adjust = PointSub(director, center);
    return atan2(adjust.y, adjust.x);
}

void GPath::drawLineJoinMiter(GCanvasContext *context, const GPoint &center,
                              const GPoint &p1, const GPoint &p2,
                              GColorRGBA color, std::vector<GVertex> *vec) {
    float angle1 = calcPointAngle(p1, center);
    float angle2 = calcPointAngle(p2, center);

    float angleGap = (angle2 - angle1);
    if (angleGap < 0) {
        angleGap += (PI_1 * 2);
    }
    angleGap /= 2;
    float miterLen = fabsf(1 / cosf(angleGap));

    if (miterLen > context->MiterLimit()) {
        context->PushTriangle(center, p1, p2, color);
        return;
    }

    float miterAngle = angle1 + angleGap;
    float lineWidth = context->LineWidth() * 0.5;
    GPoint miterPoint = {
            center.x + cosf(miterAngle) * miterLen * lineWidth,
            center.y + sinf(miterAngle) * miterLen * lineWidth};

    // LOG_E("drawLineJoinMiter PushQuad=%f,%f %f,%f %f,%f,%f,%f",
    //      center.x, center.y, p1.x, p1.y, miterPoint.x, miterPoint.y, p2.x, p2.y);

    // filter opengl invalid point( eg:oppo a59 )
    if (isNan(p1) || isNan(p2) || isNan(center) || isNan(miterPoint))
    {
        return;
    }

    context->PushQuad(center, p1, miterPoint, p2, color, vec);
}

void GPath::drawLineCap(GCanvasContext *context, const GPoint &center,
                        const GPoint &p1, const GPoint &p2, float deltaX,
                        float deltaY, GColorRGBA color, std::vector<GVertex> *vec, float samePointThreshold) {
    if (context->LineCap() == LINE_CAP_SQUARE) {
        context->PushQuad(p1, p2, PointMake(p2.x + deltaX, p2.y + deltaY),
                          PointMake(p1.x + deltaX, p1.y + deltaY), color, vec);
    } else if (context->LineCap() == LINE_CAP_ROUND) {
        drawArcToContext(context, center, p1, p2, color, vec, samePointThreshold);
    }
}

void GPath::SubdivideCubicTo(GPath *path, GPoint points[4], int level) {
    if (--level >= 0) {
        GPoint tmp[7];

        ChopCubicAt(points, tmp, 0.5f);
        SubdivideCubicTo(path, &tmp[0], level);
        SubdivideCubicTo(path, &tmp[3], level);
    } else {
        path->push(points[1]);
        path->push(points[2]);
        path->push(points[3]);
    }
}

void GPath::ChopCubicAt(GPoint src[4], GPoint dst[7], float t) {

    GPoint tt = PointMake(t, t);

    GPoint ab = interp(src[0], src[1], tt);
    GPoint bc = interp(src[1], src[2], tt);
    GPoint cd = interp(src[2], src[3], tt);
    GPoint abc = interp(ab, bc, tt);
    GPoint bcd = interp(bc, cd, tt);
    GPoint abcd = interp(abc, bcd, tt);

    dst[0] = src[0];
    dst[1] = ab;
    dst[2] = abc;
    dst[3] = abcd;
    dst[4] = bcd;
    dst[5] = cd;
    dst[6] = src[3];
}

GPoint GPath::interp(const GPoint &v0, const GPoint &v1, const GPoint &t) {
    return PointMake(v0.x + (v1.x - v0.x) * t.x, v0.y + (v1.y - v0.y) * t.y);
}

void GPath::GetRect(GRectf& rect)
{
    rect.leftTop = mMinPosition;
    rect.bottomRight = mMaxPosition;
}



void GPath::RestoreStencilForClip(GCanvasContext *context)
{
    if (context->HasClipRegion())
    {
        // reset stencil
        glClear(GL_STENCIL_BUFFER_BIT);
        SetStencilForClip();
    }
    else
    {
        // disable
        glStencilMask(0xFF);
        glClear(GL_STENCIL_BUFFER_BIT);
        glDisable(GL_STENCIL_TEST);
    }
}


void GPath::SetStencilForClip()
{
    GLuint mask = 0x80;
    glStencilMask(mask);
    glStencilFunc(GL_EQUAL, mask, mask);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

inline double calc_distance(double x1, double y1, double x2, double y2)
{
    double dx = x2-x1;
    double dy = y2-y1;
    return sqrt(dx * dx + dy * dy);
}

inline double calc_sq_distance(double x1, double y1, double x2, double y2)
{
    double dx = x2-x1;
    double dy = y2-y1;
    return dx * dx + dy * dy;
}
