# Definition of histogram variables

The `.json` files contain the definition of the variables used for binning into histograms.  
See `ewkino/Tools/python/variabletools.py` for the parsing of these files,  
allowed fields, naming conventions, etc.  

In normal circumstances, do not modify the `.txt` files manually,  
as they are generated automatically when needed (for reading in C++ instead of python).  
See `ewkino/Tools/python/variabletools.py` for how the conversion is done,  
and `ewkino/Tools/src/variableTools.cc` for the reading of the resulting `.txt` files.
