// --------------------------------------------------------------------------
//                   OpenMS -- Open-Source Mass Spectrometry
// --------------------------------------------------------------------------
// Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
// ETH Zurich, and Freie Universitaet Berlin 2002-2018.
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

#include <OpenMS/APPLICATIONS/TOPPBase.h>

#include <OpenMS/SYSTEM/File.h>
#include <OpenMS/FORMAT/MzMLFile.h>
#include <OpenMS/FORMAT/FASTAFile.h>

//-------------------------------------------------------------
// Doxygen docu
//-------------------------------------------------------------

/**
   @page TOPP_MSGFPlusAdapter MSGFPlusAdapter

   @brief Adapter for the MS-GF+ protein identification (database search) engine.

<CENTER>
    <table>
        <tr>
            <td ALIGN = "center" BGCOLOR="#EBEBEB"> pot. predecessor tools </td>
            <td VALIGN="middle" ROWSPAN=2> \f$ \longrightarrow \f$ MSGFPlusAdapter \f$ \longrightarrow \f$</td>
            <td ALIGN = "center" BGCOLOR="#EBEBEB"> pot. successor tools </td>
        </tr>
        <tr>
            <td VALIGN="middle" ALIGN = "center" ROWSPAN=1> @ref TOPP_PeakPickerHiRes @n (or another centroiding tool)</td>
            <td VALIGN="middle" ALIGN = "center" ROWSPAN=1> @ref TOPP_IDFilter or @n any protein/peptide processing tool</td>
        </tr>
    </table>
</CENTER>

    MS-GF+ must be installed before this wrapper can be used. Please make sure that Java and MS-GF+ are working.@n
    The following MS-GF+ version is required: MS-GF+ Beta (v10089) (7/31/2014). At the time of writing, it could be downloaded from http://omics.pnl.gov/software/ms-gf. Older versions will not work properly.

    Input spectra for MS-GF+ have to be centroided; profile spectra are ignored.

    The first time MS-GF+ is applied to a database (FASTA file), it will index the file contents and generate a number of auxiliary files in the same directory as the database (e.g. for "db.fasta": "db.canno", "db.cnlap", "db.csarr" and "db.cseq" will be generated). It is advisable to keep these files for future MS-GF+ searches, to save the indexing step.@n

    @note When a new database is used for the first time, make sure to run only one MS-GF+ search against it! Otherwise one process will start the indexing and the others will crash due to incomplete index files. After a database has been indexed, multiple MS-GF+ processes can use it in parallel.

    This adapter supports relative database filenames, which (when not found in the current working directory) are looked up in the directories specified by 'OpenMS.ini:id_db_dir' (see @subpage TOPP_advanced).

    The adapter works in three steps to generate an idXML file: First MS-GF+ is run on the input MS data and the sequence database, producing an mzIdentML (.mzid) output file containing the search results. This file is then converted to a text file (.tsv) using MS-GF+' "MzIDToTsv" tool. Finally, the .tsv file is parsed and a result in idXML format is generated.

    <B>The command line parameters of this tool are:</B>
    @verbinclude TOPP_MSGFPlusAdapter.cli
    <B>INI file documentation of this tool:</B>
    @htmlinclude TOPP_MSGFPlusAdapter.html
*/

// We do not want this class to show up in the docu:
/// @cond TOPPCLASSES

using namespace OpenMS;
using namespace std;

class MSGFPlusAdapter final :
  public TOPPBase
{
public:
  MSGFPlusAdapter() :
    TOPPBase("MSGFPlus", "MS/MS database search with C++ implementation of MS-GF+.", false)
  {
  }

protected:

  void registerOptionsAndFlags_() override
  {
    // Input file
    registerInputFile_(IN, "<file>", "", "Input file (MS-GF+ parameter '-s')");
    setValidFormats_(IN, ListUtils::create<String>("mzML")); // mzML,mzXML,mgf,ms2 // TODO

    // Output File
    registerOutputFile_(OUT, "<file>", "", "Output file", false);
    setValidFormats_(OUT, ListUtils::create<String>("idXML"));

    // The Database
    registerInputFile_(DATABASE, "<file>", "", "Protein sequence database (FASTA file; MS-GF+ parameter '-d'). Non-existing relative filenames are looked up via 'OpenMS.ini:id_db_dir'.", true, false, ListUtils::create<String>("skipexists"));
    setValidFormats_(DATABASE, ListUtils::create<String>("FASTA"));

    // Fragment Method
    // [-m FragmentMethodID] (0: As written in the spectrum or CID if no info (Default), 1: CID, 2: ETD, 3: HCD, 4: UVPD)
    const vector<String> fragment_methods = ListUtils::create<String>("from_spectrum,CID,ETD,HCD,UVPD");
    registerStringOption_(FRAGMENT_METHOD, "<choice>", fragment_methods[0], "Fragmentation method ('from_spectrum' relies on spectrum meta data and uses CID as fallback option; MS-GF+ parameter '-m')", false);
    setValidStrings_(FRAGMENT_METHOD, fragment_methods);

    // Minimum precursor charge
    const int min_allowed_precursor_charge = 1;
    registerIntOption_(MIN_PRECURSOR_CHARGE, "<num>", 2, "Minimum precursor ion charge (only used for spectra without charge information; MS-GF+ parameter '-minCharge')", false);
    setMinInt_(MIN_PRECURSOR_CHARGE, min_allowed_precursor_charge);

    // Maximum precursor charge
    registerIntOption_(MAX_PRECURSOR_CHARGE, "<num>", 3, "Maximum precursor ion charge (only used for spectra without charge information; MS-GF+ parameter '-maxCharge')", false);
    setMinInt_(MAX_PRECURSOR_CHARGE, min_allowed_precursor_charge);

    // Precursor mass tolerance with unit
    registerDoubleOption_(PRECURSOR_MASS_TOLERANCE, "<value>", 10, "Precursor monoisotopic mass tolerance (MS-GF+ parameter '-t')", false);
    setMinFloat_(PRECURSOR_MASS_TOLERANCE, 0.0);
    registerStringOption_(PRECURSOR_ERROR_UNIT, "<choice>", "ppm", "Unit of precursor mass tolerance (MS-GF+ parameter '-t')", false);
    setValidStrings_(PRECURSOR_ERROR_UNIT, ListUtils::create<String>("Da,ppm"));
  }

  ExitCodes main_(int, const char**) override
  {
    cout << "Hello World" << endl;

    /*
     * Reading of required parameters
     */
    // Input File
    const String &input_file = getStringOption_(IN);
    if ( ! File::readable(input_file)) {
        writeLog_("Fatal: Input File : " + input_file + " either does not exist or is not readable!");
        return ILLEGAL_PARAMETERS;
    }

    // Output File
    const String &output_file = getStringOption_(OUT);
    if (output_file.empty())
    {
      writeLog_("Fatal: No output file given (parameter 'out' or 'mzid_out')");
      return ILLEGAL_PARAMETERS;
    }

    // Database
    // TODO Currently unused
    const String &database_file = getStringOption_(DATABASE);
    if ( ! File::readable(database_file))
    {
      writeLog_("Fatal: Provided database file: " + database_file + " is not readable!");
      return ILLEGAL_PARAMETERS;
    }

    // Precursor charges
    // TODO Currently unused
    const int min_precursor_charge = getIntOption_(MIN_PRECURSOR_CHARGE);
    const int max_precursor_charge = getIntOption_(MAX_PRECURSOR_CHARGE);
    if (min_precursor_charge > max_precursor_charge)
    {
      writeLog_("Fatal: min_precursor_charge cannot be larger than max_precursor_charge");
      return ILLEGAL_PARAMETERS;
    }

    // Precursor mass tolerance with unit
    // TODO Currently unused
    const double &precusor_mass_tolerance = getDoubleOption_(PRECURSOR_MASS_TOLERANCE);
    const String &precursor_error_unit = getStringOption_(PRECURSOR_ERROR_UNIT);
    //

    /*
     * MSGFPlus algorithm
     */
    // 1. Load experiment
    PeakMap input_map;
    MzMLFile().load(input_file, input_map);

//    // 2. Get map of precursor charges for all spectra
    map<UInt, int> precursor_charges;

    // TODO Use activation method
    // TODO Precursor charge = 0
    // TODO Ignore spectra that have too few of peaks
    // TODO Ignore profile spectra
    computeChargeMap_(input_map, min_precursor_charge, max_precursor_charge, precursor_charges);
//    for (auto const &pair : precursor_charges)
//    {
//        cout << pair.first << ':' << pair.second << endl;
//    }
    if (precursor_charges.empty())
    {
      writeLog_("Spectrum file " + input_file + " does not have any valid spectra. No output.");
      return INPUT_FILE_EMPTY;
    }
    writeLog_("Reading Spectra finished\nProcessing a total of " + String(precursor_charges.size()) + " spectra");

    // TODO Write parameters if verbose mode is turned on

    // TODO TDA

//    // 3. Counts the frequencies of AAs in the database
//    map<char, UInt> amino_acid_freqs;
//    const UInt num_aa_in_database = countAminoAcids_(database_file, amino_acid_freqs);


    return EXECUTION_OK;
  }

private:
    // Parameter names
    const String IN = "in";
    const String OUT = "out";
    const String FRAGMENT_METHOD = "fragment_method";
    const String MIN_PRECURSOR_CHARGE = "min_precursor_charge";
    const String MAX_PRECURSOR_CHARGE = "max_precursor_charge";
    const String DATABASE = "database";
    const String PRECURSOR_MASS_TOLERANCE = "precursor_mass_tolerance";
    const String PRECURSOR_ERROR_UNIT = "precursor_error_unit";

    // Constants
    const Size MIN_NUM_PEAKS_PER_SPECTRUM = 10;

    /*
     * Properties of the experiment
     */
    Size num_spectra_with_too_few_peaks = 0;


    // Maps spectrum identifier to charge map
    // TODO Check if correct
    // TODO CHeck charges
    // TODO check min num peaks per spectrum
    void computeChargeMap_(
            const PeakMap &exp,
            const int min_precursor_charge,
            const int max_precursor_charge,
            map<UInt, int> &precursor_charges)
    {
      UInt index = 0;
      for (const MSSpectrum &spectrum : exp)
      {
       index++;
       const vector<Precursor> &precursors = spectrum.getPrecursors();
       if ( ! precursors.empty())
       {
         const int charge = precursors[0].getCharge();
         precursor_charges[index] = charge;
       }
      }
    }

    UInt countAminoAcids_(const String &database_file, map<char, UInt> &amino_acid_freqs) const
    {
      FASTAFile file;
      file.readStart(database_file);
      FASTAFile::FASTAEntry entry;
      unsigned int total = 0;
      while (file.readNext(entry))
      {
        for (const char c : entry.sequence)
        {
          total++;
          amino_acid_freqs[c]++;
        }
      }
      return total;
    }
};


int main(int argc, const char** argv)
{
  MSGFPlusAdapter tool;
  return tool.main(argc, argv);
}
