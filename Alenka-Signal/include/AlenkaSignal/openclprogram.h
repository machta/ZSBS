#ifndef ALENKASIGNAL_OPENCLPROGRAM_H
#define ALENKASIGNAL_OPENCLPROGRAM_H

#ifdef __APPLE__
#include <OpenCL/cl_gl.h>
#else
#include <CL/cl_gl.h>
#endif

#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

namespace AlenkaSignal {

class OpenCLContext;

/**
 * @brief A wrapper for cl_program.
 */
class OpenCLProgram {
  cl_program program;
  bool invalid;
  OpenCLContext *context;

public:
  /**
   * @brief OpenCLProgram constructor.
   * @param source The source string.
   */
  OpenCLProgram(const std::string &source, OpenCLContext *context);
  OpenCLProgram(const std::vector<unsigned char> *binary,
                OpenCLContext *context);
  ~OpenCLProgram();

  /**
   * @brief Returns a kernel object.
   * @param kernelName The name of the kernel function.
   * @return Returns nullptr if the compilation failed.
   *
   * The returned kernel object is independent of this class and the caller
   * takes its ownership.
   */
  cl_kernel createKernel(const std::string &kernelName) const;

  /**
   * @brief Returns cl_program compilation status.
   * @return True if there was no error during compilation.
   */
  bool compileSuccess() const { return !invalid; }

  /**
   * @brief Returns a string with the compilation output (errors and warnings).
   */
  std::string getCompileLog() const;

  std::vector<unsigned char> *getBinary();

private:
  void build();
};

} // namespace AlenkaSignal

#endif // ALENKASIGNAL_OPENCLPROGRAM_H
