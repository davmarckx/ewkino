# Definition of histogram variables

## Introduction

The `.json` files contain the definition of the variables used for binning into histograms.  
See `ewkino/Tools/python/variabletools.py` for the parsing of these files,  
allowed fields, naming conventions, etc.  

In normal circumstances, do not modify the `.txt` files manually,  
as they are generated automatically when needed (for reading in C++ instead of python).  
See `ewkino/Tools/python/variabletools.py` for how the conversion is done,  
and `ewkino/Tools/src/variableTools.cc` for the reading of the resulting `.txt` files.  

## List of individual variable json files

- `variables_test.json`: just a dummy collection of variables for testing.  
- `variables_copyfromtzq.json`: variables copied (with needed style modifications) from the tZq analysis.  
  (in detail, from here: `https://github.com/LukaLambrecht/ewkino/blob/tZq_new/tZqAnalysisCode/systematics/runsystematics.cc`  
   and here: `https://github.com/LukaLambrecht/ewkino/blob/tZq_new/tZqAnalysisCode/plotting/histplotter_postfit.py`).  
  They were automatically parsed using `parse_variables.py`.
