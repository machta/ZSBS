#ifndef ALENKASIGNAL_MONTAGE_H
#define ALENKASIGNAL_MONTAGE_H

#include "openclprogram.h"

#ifdef __APPLE__
#include <OpenCL/cl_gl.h>
#else
#include <CL/cl_gl.h>
#endif

#include <memory>
#include <string>

namespace AlenkaSignal {

enum MontageType { NormalMontage, IdentityMontage, CopyMontage };

/**
 * @brief A class for creating kernel programs for montage computation.
 *
 * This class generates a source string for a kernel function called 'montage'.
 * For this it uses the formulas passed to it in the constructor, the
 * montageHeader.cl file and string constants hard-coded in the cpp file.
 *
 * Then this string is used for creating an OpenCL kernel object that is
 * the final representation of the code. After the kernel object is retrieved
 * this object can be safely destroyed.
 *
 * @todo Prohibit copying of this object.
 */
template <class T> class Montage {
  OpenCLContext *context = nullptr;
  MontageType montageType = NormalMontage;
  std::string source;
  std::unique_ptr<OpenCLProgram> program;
  cl_kernel kernel = nullptr;
  cl_int copyIndex = -1;

public:
  /**
   * @brief Montage constructor.
   * @param sources OpenCL source code of the montage.
   */
  Montage(const std::string &source, OpenCLContext *context,
          const std::string &headerSource = "",
          const std::vector<std::string> &labels = std::vector<std::string>());
  Montage(const std::vector<unsigned char> *binary, OpenCLContext *context)
      : program(new OpenCLProgram(binary, context)) {}
  ~Montage();

  /**
   * @brief Returns the kernel object needed for execution of the code.
   */
  cl_kernel getKernel() {
    buildProgram();
    if (!kernel)
      kernel = program->createKernel("montage");
    return kernel;
  }

  std::vector<unsigned char> *getBinary() {
    buildProgram();
    if (program)
      return program->getBinary();
    else
      return nullptr;
  }

  MontageType getMontageType() const { return montageType; }
  cl_int copyMontageIndex() const { return copyIndex; }

  std::string getSource() const { return source; }

  /**
   * @brief Tests the source code of the montage.
   * @param source String formula for one track.
   * @param errorMessage [out] If not nullptr and an error is detected, an error
   * message is stored here.
   * @return True if the test succeeds.
   */
  static bool test(const std::string &source, OpenCLContext *context,
                   std::string *errorMessage = nullptr);
  static bool testHeader(const std::string &source, OpenCLContext *context,
                         const std::string &headerSource,
                         std::string *errorMessage = nullptr);

  /**
   * @brief Removes single line and block comments from OpenCL code.
   */
  static std::string stripComments(const std::string &code);

private:
  std::string preprocessSource(const std::string &source,
                               const std::vector<std::string> &labels);
  void buildProgram();
  void buildCopyProgram();
  void buildIdentityProgram();
};

} // namespace AlenkaSignal

#endif // ALENKASIGNAL_MONTAGE_H
