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
// --------------------------------------------------------------------------
// $Maintainer: Lukas Zimmermann $
// $Authors: Lukas Zimmermann $
// --------------------------------------------------------------------------

#include <OpenMS/config.h>
#include <OpenMS/APPLICATIONS/TOPPBase.h>
#include <OpenMS/FORMAT/XQuestResultXMLFile.h>
#include <OpenMS/METADATA/XQuestResultMeta.h>

using namespace OpenMS;
using namespace std;

//-------------------------------------------------------------
//Doxygen docu
//-------------------------------------------------------------

/**
    @page UTILS_XMLValidator XMLValidator

    @brief Validates XML files against an XSD schema.

    When a schema file is given, the input file is simply validated against the schema.

    When no schema file is given, the tool tries to determine the file type and
    validates the file against the latest schema version.

    @note XML schema files for the %OpenMS XML formats and several other XML
    formats can be found in the folder
          OpenMS/share/OpenMS/SCHEMAS/

    <B>The command line parameters of this tool are:</B>
    @verbinclude UTILS_XMLValidator.cli
    <B>INI file documentation of this tool:</B>
    @htmlinclude UTILS_XMLValidator.html
*/

// We do not want this class to show up in the docu:
/// @cond TOPPCLASSES

class TOPPXLConverter :
  public TOPPBase
{
  static const String param_input_file;
  static const String param_output_file;


public:
  TOPPXLConverter() :
    TOPPBase("XLConverter", "Converts files containing cross-link information in various different other formats.", false)
  {
  }

protected:

  void registerOptionsAndFlags_()
  {
    // Input file
    registerInputFile_(TOPPXLConverter::param_input_file, "<input_file>", "", "Input file with cross-link information to be converted", true, false);
    setValidFormats_(TOPPXLConverter::param_input_file, ListUtils::create<String>("xml"));

    // Which program should be able to read the provided output file

    // Output file
    registerOutputFile_(TOPPXLConverter::param_output_file, "<output_file>", "", "Output file", true, false);
    setValidFormats_(TOPPXLConverter::param_output_file, ListUtils::create<String>("csv"));

  }

  ExitCodes main_(int, const char**)
  {

    String arg_input_file = getStringOption_(TOPPXLConverter::param_input_file);

    // Handle xQuest input file
    if (arg_input_file.hasSuffix("xml"))
    {
      XQuestResultXMLFile input_file;
      vector< XQuestResultMeta > metas;
      vector < vector < PeptideIdentification > > spectra;
      input_file.load(arg_input_file, metas, spectra, false, 1, false);




    }



    return EXECUTION_OK;
  }

};

const String TOPPXLConverter::param_input_file = "input_file";
const String TOPPXLConverter::param_output_file = "output_file";


int main(int argc, const char** argv)
{
  TOPPXLConverter tool;
  return tool.main(argc, argv);
}

/// @endcond
