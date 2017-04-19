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
#include <OpenMS/DATASTRUCTURES/StringUtils.h>
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

    static const String param_executable;
    static const String param_spectra_files;  // From which the library is to be generated
    static const String param_spectra_files_formats;  // File formats being allowed for the input spectra
    static const String param_params_file;  // Parameter file for library creation
    static const String param_library_file;   // Library output

    // Further parameters
    static const String param_remark;
    static const String param_print_MRM_table;
    static const String param_min_prob;
    static const double param_min_prob_default;
    static const String param_max_fdr;
    static const double param_max_fdr_default;
    static const String param_dataset_name;

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

      // Handle executable
      registerInputFile_(TOPPSpectraSTCreateAdapter::param_executable, "<path>", "spectrast", "Path to the SpectraST executable to use; may be empty if the executable is globally available.", true, false, ListUtils::create<String>("skipexists"));

      // Register input files for Spectra creation
      registerInputFileList_(TOPPSpectraSTCreateAdapter::param_spectra_files, "<FileName1> [ <FileName2> ... <FileNameN> ]", empty, "File names(s) of spectra from which the library should be created.", true, false);
      setValidFormats_(TOPPSpectraSTCreateAdapter::param_spectra_files, ListUtils::create<String>(TOPPSpectraSTCreateAdapter::param_spectra_files_formats), false);

      // params file
      registerInputFile_(TOPPSpectraSTCreateAdapter::param_params_file, "<params_file>", "", "Read create options from file. All options set in the file will be overridden by command-line options if specified.", false, false);
      setValidFormats_(TOPPSpectraSTCreateAdapter::param_params_file, ListUtils::create<String>("params"), false);

      // library (output) file
      registerOutputFile_(TOPPSpectraSTCreateAdapter::param_library_file, "<library_file>", "", "Output library file", true, false);
      setValidFormats_(TOPPSpectraSTCreateAdapter::param_library_file, ListUtils::create<String>("splib"), false);

      // Remark
      registerStringOption_(TOPPSpectraSTCreateAdapter::param_remark, "<remark>", "", "Add a Remark=<remark> comment to all library entries created.", false, false);

      // Whether to print MRM table
      registerFlag_(TOPPSpectraSTCreateAdapter::param_print_MRM_table, "Whether to write all library spectra as SRM transition tables.", true);

      // Min probability of spectra
      registerDoubleOption_(TOPPSpectraSTCreateAdapter::param_min_prob, "<min_prob>", TOPPSpectraSTCreateAdapter::param_min_prob_default, "Include all spectra identified with probability no less than <min_prob> in the library.", false, true);

      // Max FDR
      registerDoubleOption_(TOPPSpectraSTCreateAdapter::param_max_fdr, "<max_fdr>", TOPPSpectraSTCreateAdapter::param_max_fdr_default, "(Only for pepXML import) Include spectra with global FDR no greater than <fdr> the library.	", false, true);

      // Dataset name
      registerStringOption_(TOPPSpectraSTCreateAdapter::param_dataset_name, "<dataset_name>", "", "Specify a dataset identifier for the file to be imported.", false, false);
    }


    // the main_ function is called after all parameters are read
    ExitCodes main_(int, const char **)
    {
      //--------------------------------------------------------
      // Figure out the input format of the spectra input files
      //--------------f------------------------------------------
      StringList spectra_files = getStringList_(TOPPSpectraSTCreateAdapter::param_spectra_files);
      if (spectra_files.size() < 1)
      {
        LOG_ERROR << "ERROR: At least one input file containing spectra must be provided" << endl;
        return ILLEGAL_PARAMETERS;
      }
      String first_spectra_file = spectra_files[0];

      // Figure out the input file format from the first provided file
      StringList valid_spectra_formats;
      StringUtils::split(TOPPSpectraSTCreateAdapter::param_spectra_files_formats, ",", valid_spectra_formats);
      for (StringList::const_iterator valid_spectra_formats_it = valid_spectra_formats.begin();
           valid_spectra_formats_it != valid_spectra_formats.end(); ++valid_spectra_formats_it)
      {
        String current_format = *valid_spectra_formats_it;
        if (first_spectra_file.hasSuffix(current_format))
        {
          this->input_format = current_format;
          break;
        }
      }
      // Exit if spectra format has not been recognized
      if (this->input_format.empty())
      {
        LOG_ERROR << "ERROR: Unrecognized input format from spectra file: " << first_spectra_file << endl;
        return ILLEGAL_PARAMETERS;
      }

      // Assemble command line for SpectraST
      QStringList arguments;

      // Executable
      String executable = getStringOption_(TOPPSpectraSTCreateAdapter::param_executable);
      if (executable.empty())
      {
          executable = "spectrast";
      }

      // Make create mode explicit
      arguments << "-c";

      // Set the parameter file if present
      String params_file = getStringOption_(TOPPSpectraSTCreateAdapter::param_params_file);
      if ( ! params_file.empty())
      {
          arguments << params_file.toQString().prepend("-cF");
      }

      // Set the library (output) file
      String library_file = getStringOption_(TOPPSpectraSTCreateAdapter::param_library_file);
      if (library_file.empty())
      {
        LOG_ERROR << "ERROR: File for output library has not been specified. Terminating." << endl;
        return ILLEGAL_PARAMETERS;
      }
      if ( ! library_file.hasSuffix("splib"))
      {
        LOG_ERROR << "ERROR: File for output library does not have the correct file ending (splib). Terminating." << endl;
        return ILLEGAL_PARAMETERS;
      }

      // For spectrast, the file extension has to be removed from the library file
      arguments << File::removeExtension(library_file).toQString().prepend("-cN");

      // Parameter remark
      String param_remark = getStringOption_(TOPPSpectraSTCreateAdapter::param_remark);
      if ( ! param_remark.empty())
      {
        arguments << param_remark.toQString().prepend("-cm");
      }

      // Parameter print MRM table
      arguments << (getFlag_(TOPPSpectraSTCreateAdapter::param_print_MRM_table) ? "-cM" : "-cM!");

      // Parameter min_prob
      double min_prob = getDoubleOption_(TOPPSpectraSTCreateAdapter::param_min_prob);
      if (min_prob < 0 || min_prob > 1)
      {
        LOG_ERROR << "ERROR: Values for parameter -min_prob larger than 1 or less than 0 are not allowed. Terminating." << endl;
        return ILLEGAL_PARAMETERS;
      }
      arguments << QString::number(min_prob).prepend("-cP");

      // Parameter max_fdr
      double max_fdr = getDoubleOption_(TOPPSpectraSTCreateAdapter::param_max_fdr);
      if (max_fdr < 0)
      {
        LOG_ERROR << "ERROR: Values for parameter -max_fdr less than 0 are not allowed. Terminating." << endl;
        return ILLEGAL_PARAMETERS;
      }
      arguments << QString::number(max_fdr).prepend("-cq");

      // Parameter Dataset name
      String dataset_name = getStringOption_(TOPPSpectraSTCreateAdapter::param_dataset_name);
      if ( ! dataset_name.empty())
      {
        arguments << dataset_name.toQString().prepend("-cn");
      }

      // Add all input files. Make sure that the file ending is the same as defined from the first provided file
      for (StringList::const_iterator spectra_files_it = spectra_files.begin();
           spectra_files_it != spectra_files.end(); ++spectra_files_it)
      {
        String spectra_file = *spectra_files_it;
        if ( ! spectra_file.hasSuffix(this->input_format))
        {
          LOG_ERROR << "ERROR: Input spectra file does not agree in format: "
                         << spectra_file << " is not " << this->input_format << endl;
          return ILLEGAL_PARAMETERS;
        }
        arguments << spectra_file.toQString();
      }

      // Write command-line call to DEBUG_LOG
      cout << "COMMAND: " << executable;
      for (QStringList::const_iterator it = arguments.begin(); it != arguments.end(); ++it)
      {
          cout << " " << it->toStdString();
      }
      cout << endl;


      // Run spectrast
      QProcess spectrast_process;
      spectrast_process.start(executable.toQString(), arguments);
      if (! spectrast_process.waitForFinished(-1))
      {
          LOG_ERROR << "Fatal error running SpectraST\nDoes the spectrast executable exist?" << endl;
          return EXTERNAL_PROGRAM_ERROR;
      }


      // Exit the tool
      return EXECUTION_OK;
    }
private:

    String input_format;

    /**
     * @brief Checks whether the input_format is part of the extension list given in @p file_list
     * @param List of file extensions @p file_list
     * @return Whether the currently_determined input_format is part of the file list @p file_list
     */
    inline bool inputFormatIsIn(const String & file_list)
    {
      StringList l = ListUtils::create<String>(file_list);
      return std::find(l.begin(), l.end(), this->input_format) != l.end();
    }

};
// End of Tool definition

const String TOPPSpectraSTCreateAdapter::param_executable = "executable";
const String TOPPSpectraSTCreateAdapter::param_spectra_files = "spectra_files";
const String TOPPSpectraSTCreateAdapter::param_spectra_files_formats = "msp,hlf,pepXML,pep.xml,xml,ms2,splib";
const String TOPPSpectraSTCreateAdapter::param_params_file = "params_file";
const String TOPPSpectraSTCreateAdapter::param_library_file = "library_file";
const String TOPPSpectraSTCreateAdapter::param_remark = "remark";
const String TOPPSpectraSTCreateAdapter::param_print_MRM_table = "print_MRM_table";
const String TOPPSpectraSTCreateAdapter::param_min_prob = "min_prob";
const double TOPPSpectraSTCreateAdapter::param_min_prob_default = 0.9;
const String TOPPSpectraSTCreateAdapter::param_max_fdr = "max_fdr";
const double TOPPSpectraSTCreateAdapter::param_max_fdr_default = 9999;
const String TOPPSpectraSTCreateAdapter::param_dataset_name;

// the actual main function needed to create an executable
int main(int argc, const char ** argv)
{
  TOPPSpectraSTCreateAdapter tool;
  return tool.main(argc, argv);
}
