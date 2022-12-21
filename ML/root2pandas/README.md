These scripts can be used to convert root files to pandas dataframes, saved as pickle files.


Currently, it is recommended to use the eventflattener outputs for converting to dataframes as the multiindex dataframes are not yet implemented in this framework (hard to make compatible with ML models) and conversion from a multiindex to a flat dataframe is not yet implemented.
