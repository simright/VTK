/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOpenGLRenderTimer.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

/**
 * @class   vtkOpenGLRenderTimer
 * @brief   Asynchronously measures GPU execution time for a single event.
 *
 *
 * This class posts events to the OpenGL server to measure execution times
 * of GPU processes. The queries are asynchronous and multiple
 * vtkOpenGLRenderTimers may overlap / be nested.
 *
 * This uses GL_TIMESTAMP rather than GL_ELAPSED_TIME, since only one
 * GL_ELAPSED_TIME query may be active at a time. Since GL_TIMESTAMP is not
 * available on OpenGL ES, timings will not be available on those platforms.
 * Use the static IsSupported() method to determine if the timer is availble.
 */

#ifndef vtkOpenGLRenderTimer_h
#define vtkOpenGLRenderTimer_h

#include "vtkRenderingOpenGL2Module.h" // For export macro
#include "vtkType.h" // For vtkTypeUint64, etc

class VTKRENDERINGOPENGL2_EXPORT vtkOpenGLRenderTimer
{
public:
  vtkOpenGLRenderTimer();
  ~vtkOpenGLRenderTimer();

  /**
   * Returns true if timer events are supported by the current OpenGL
   * implementation.
   */
  static bool IsSupported();

  /**
   * Clear out any previous results and prepare for a new query.
   */
  void Reset();

  /**
   * Mark the start of a timed event.
   */
  void Start();

  /**
   * Mark the end of a timed event.
   */
  void Stop();

  /**
   * Returns true if the timer has been started. The query may not be ready yet.
   */
  bool Started();

  /**
   * Returns true if the timer has been stopped. The query may not be ready yet.
   */
  bool Stopped();

  /**
   * Returns true when the timing results are available.
   */
  bool Ready();

  //@{
  /**
   * If Ready() returns true, get the elapsed time in the requested units.
   */
  float GetElapsedSeconds();
  float GetElapsedMilliseconds();
  vtkTypeUInt64 GetElapsedNanoseconds();
  //@}

  /**
   * If Ready() returns true, return the start or stop time in nanoseconds.
   * @{
   */
  vtkTypeUInt64 GetStartTime();
  vtkTypeUInt64 GetStopTime();
  /**@}*/

  /**
   * Simply calls Reset() to ensure that query ids are freed. All stored timing
   * information will be lost.
   */
  void ReleaseGraphicsResources();

protected:
  bool StartReady;
  bool EndReady;

  vtkTypeUInt32 StartQuery;
  vtkTypeUInt32 EndQuery;

  vtkTypeUInt64 StartTime;
  vtkTypeUInt64 EndTime;

private:
  vtkOpenGLRenderTimer(const vtkOpenGLRenderTimer&) VTK_DELETE_FUNCTION;
  void operator=(const vtkOpenGLRenderTimer&) VTK_DELETE_FUNCTION;
};

#endif // vtkOpenGLRenderTimer_h

// VTK-HeaderTest-Exclude: vtkOpenGLRenderTimer.h
