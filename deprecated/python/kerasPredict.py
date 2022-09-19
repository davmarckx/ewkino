from keras import models 
import numpy as np


class kerasModel():
    def __init__(self, file_name):
        self.model = models.load_model( file_name )


    #return model prediction given a list of inputs
    def predict( self, x ):
        x = np.asarray(x, dtype=float)
        x = x.reshape( (1, len(x)) )
        return float( self.model.predict(x) )



if __name__ == '__main__':
    pass

