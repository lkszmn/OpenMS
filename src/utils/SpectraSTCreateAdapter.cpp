// --------------------------------------------------------------------------
//                   OpenMS -- Open-Source Mass Spectrometry
// --------------------------------------------------------------------------
// Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
// ETH Zurich, and Freie Universitaet Berlin 2002-2016.
//
// This software is released under a three-clause BSD license:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of any author or any participating institution
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
// For a full list of authors, refer to the file AUTHORS.
// --------------------------------------------------------------------------
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL ANY OF THE AUTHORS OR THE CONTRIBUTING
// INSTITUTIONS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------
// $Maintainer: Lukas Zimmermann $
// $Authors: Lukas Zimmermann $
// --------------------------------------------------------------------------
#include <OpenMS/APPLICATIONS/TOPPBase.h>
#include <QtCore/QProcess>
#include <iostream>
#include <OpenMS/FORMAT/MzDataFile.h>
#include <OpenMS/FORMAT/MzMLFile.h>
#include <OpenMS/FORMAT/FileHandler.h>
#include <OpenMS/SYSTEM/File.h>
#include <QtCore/QProcess>
#include <QDir>

using namespace OpenMS;
using namespace std;

//-------------------------------------------------------------
// Doxygen docu
//-------------------------------------------------------------

/**
    @page UTILS_SpectraSTCreateAdapter

    @brief This util provides an interface to the 'CREATE' mode of the SpectraST program.
           All non-advanced parameters of the executable of SpectraST were translated into
           parameters of this util.

    SpectraST: Version: 5

    <B>The command line parameters of this tool are:</B>
    @verbinclude UTILS_SpectraSTCreateAdapter.cli
    <B>INI file documentation of this tool:</B>
    @htmlinclude UTILS_SpectraSTCreateAdapter.html
*/

// We do not want this class to show up in the docu:
/// @cond TOPPCLASSES

class TOPPSpectraSTCreateAdapter :
  public TOPPBase
{
 public:
  // Define parameter name

  TOPPSpectraSTCreateAdapter() :
    TOPPBase("SpectraSTCreateAdapter", "Interface to the CREATE Mode of the SpectraST executable", false)
  {
  }

protected:

  // this function will be used to register the tool parameters
  // it gets automatically called on tool execution
  void registerOptionsAndFlags_()
  {
      StringList empty;

  }


  // the main_ function is called after all parameters are read
  ExitCodes main_(int, const char **)
  {


    // Exit the tool
    return EXECUTION_OK;
  }
};
// End of Tool definition





// the actual main function needed to create an executable
int main(int argc, const char ** argv)
{
  TOPPSpectraSTCreateAdapter tool;
  return tool.main(argc, argv);
}
