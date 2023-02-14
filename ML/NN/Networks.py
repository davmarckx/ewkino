import torch
inputs = 95


#math and data packages
import pandas as pd
import numpy as np
import random
import math
import pickle

#sklearn imports
import sklearn
from sklearn.model_selection import train_test_split, KFold

from sklearn import metrics
from sklearn.metrics import roc_curve, auc
from sklearn.metrics import roc_auc_score
from sklearn import __version__

# imports from pytorch
from torch import nn
from torch.utils.data import DataLoader
import torch.optim as optim
import torch.nn.functional as F
from torchviz import make_dot
from torch.nn import Linear


from torch_geometric.nn import GCNConv
from torch_geometric.nn import GraphConv
from torch_geometric.loader import DataLoader
from torch_geometric.nn import global_mean_pool
from torch_geometric.nn import GATConv
from torch_geometric.nn import GraphNorm


#plotting packages
import seaborn as sns
import matplotlib.pyplot as plt

################# accuracy metrics ########################################

def average_precision_cv(model, seed):
    ## Set random seeds for reproducibility
    random.seed(seed)
    np.random.seed(seed)

    ## Division into folds
    kf = KFold(5, shuffle=True, random_state=seed)

    ## STANDARDSCALER APPLIED!!!
    clf = make_pipeline(preprocessing.StandardScaler(), model)
    
    ## Compute SPECIFIC error
    #prec = cross_val_score(model, X.values, y, scoring='average_precision', cv=kf)
    balanced_prec = cross_val_score(clf, X.values, y, scoring='balanced_accuracy', cv=kf) #kan switchen met scoring=... https://scikit-learn.org/stable/modules/model_evaluation.html
    return( balanced_prec)

def binary_acc(y_pred, y_test):
    y_pred_tag = torch.round(torch.sigmoid(y_pred))

    correct_results_sum = (y_pred_tag == y_test).sum().float()
    acc = correct_results_sum/y_test.shape[0]
    acc = torch.round(acc * 100)
    
    return acc
###########################################################################


################### class to convert dataset into tensors #################
class Dataset(torch.utils.data.Dataset):

    def __init__(self, X, y, y_reg=None,regression=False,weightage=None,Multiclass=False):
        self.y_reg = None
        self.weight=None
        if not torch.is_tensor(X):
            self.X = torch.from_numpy(np.array(X))
        else:
            self.X = X
            
        outs = np.array(y)
        newouts = []
        if regression == False and Multiclass == False:
            for value in outs:
                if value == 1:
                    newouts.append([0, 1])
                elif value == 0:
                    newouts.append([1, 0])
            self.y = torch.from_numpy(np.array(newouts))
            if not y_reg is None:
                self.y_reg = torch.from_numpy(np.array(y_reg))
            if not weightage is None:
                self.weight = torch.from_numpy(np.array(weightage))

        elif regression == False and Multiclass == True:
            self.y = torch.from_numpy(outs)
            if not y_reg is None:
                self.y_reg = torch.from_numpy(np.array(y_reg))
            if not weightage is None:
                self.weight = torch.from_numpy(np.array(weightage))

        else:
            if not torch.is_tensor(X):
                self.X = torch.from_numpy(np.array(X))
            else:
                self.X = X
            if not torch.is_tensor(y):
                self.y = torch.from_numpy(np.array(y))
            else:
                self.y = y
            if not y_reg is None:
                self.y_reg = torch.from_numpy(np.array(y_reg))
                self.weight = torch.from_numpy(np.array(weightage))

    def __len__(self):
        return len(self.y)

    def __getitem__(self, i):
        if self.y_reg is None:
            if self.weight is None:
                return self.X[i], self.y[i]
            else:
                return self.X[i], self.y[i], self.weight[i]
        else:
            return self.X[i], self.y[i], self.y_reg[i],self.weight[i]
        




##################### MODELS ##########################################

# MODEL 1:
class SmallNetwork(nn.Module):
    def __init__(self,dropval):
        super().__init__()
        self.drop = nn.Dropout(p=dropval)
        self.Linear1 = nn.Linear(inputs, 100)
        self.Linear2 = nn.Linear(inputs, 90)
        self.Bilinear = nn.Bilinear(100,90,100)
        self.layers = nn.Sequential(
          nn.Linear(100, 2),
          nn.Sigmoid()
        )


    def forward(self, x):
        x1 = F.relu(self.drop(self.Linear1(x)))
        #x1 = self.drop(x1)
        x2 = F.relu(self.drop(self.Linear2(x)))
        #x2 = self.drop(x2)
        x = F.relu(self.drop(self.Bilinear(x1,x2)))
        return self.layers(x)

# MODEL 2:
class SmallNetwork2(nn.Module):
    def __init__(self,dropval):
        super().__init__()
        self.drop = nn.Dropout(p=dropval)
        self.Linear1 = nn.Linear(inputs, 200)
        self.Linear2 = nn.Linear(200, 100)
        self.layers = nn.Sequential(
          nn.Linear(100, 2),
          nn.Sigmoid(),
        )


    def forward(self, x):
        x = self.Linear1(x)
        x = self.drop(x)
        x = F.relu(x)
        x = self.Linear2(x)
        x = self.drop(x)
        x = F.relu(x)
        return self.layers(x)

# MODEL 1:
class SmallNetwork(nn.Module):
    def __init__(self,dropval):
        super().__init__()
        self.drop = nn.Dropout(p=dropval)
        self.Linear1 = nn.Linear(inputs, 100)
        self.Linear2 = nn.Linear(100, 100)
        self.Bilinear = nn.Bilinear(100,100,100)
        self.Bilinear2 = nn.Bilinear(100,100,100)
        self.layers = nn.Sequential(
          nn.Linear(100, 2),
          nn.Sigmoid()
        )


    def forward(self, x):
        x = F.relu(self.drop(self.Linear1(x)))
        #x1 = self.drop(x1)
        x = F.relu(self.drop(self.Bilinear(x,x)))
        x = F.relu(self.drop(self.Bilinear2(x,x)))
        
        
        return self.layers(x)

# MODEL for regression
class SmallNetwork3(nn.Module):
    def __init__(self,dropval):
        super().__init__()
        self.drop = nn.Dropout(p=regdropval)
        self.Linear1 = nn.Linear(inputs, 60)
        self.Linear2 = nn.Linear(60, 40)
        self.layers = nn.Sequential(
          nn.Linear(40,1)
        )


    def forward(self, x):
        x = F.relu(self.drop(self.Linear1(x)))
        x = F.relu(self.drop(self.Linear2(x)))
        return self.layers(x)

# OLD !!!!!   
class BigNetwork(nn.Module):
    def __init__(self,dropval):
        super().__init__()
        self.drop = nn.Dropout(p=dropval)
        self.regdrop = nn.Dropout(p=regdropval)
        self.Linear1 = nn.Linear(inputs, 100)
        self.Linear2 = nn.Linear(inputs, 90)
        self.Bilinear = nn.Bilinear(100,90,100)
        self.layers1 = nn.Sequential(
          nn.Linear(100, 2),
          nn.Sigmoid()
        )
        self.Linear3 = nn.Linear(2, 50)
        self.layers2 = nn.Sequential(
          nn.Linear(50, 30),
          F.relu(),              # dropout of last layer turned off
          nn.Linear(30,1)
        )


    def forward(self, x):
        x1 = F.relu(self.drop(self.Linear1(x)))
        x2 = F.relu(self.drop(self.Linear2(x)))
        x = F.relu(self.drop(self.Bilinear(x1,x2)))
        x = self.layers1(x)
        x3 = grad_reverse(x)
        x3 = F.relu(self.regdrop(self.Linear3(x3)))
        x3 = self.layers2(x3)
        return x,x3            # returns classification prediction and regression prdeiction, regression layers2 should be remodeled for each feature we want to decorrelate

class GCN(torch.nn.Module):
    def __init__(self, dropval, dataset,nheads=1,self_loops=True,graphnorm=True):
        super().__init__()
        self.conv1 = GATConv(dataset.num_node_features, 200, heads=nheads,edge_dim=1,add_self_loops=True,negative_slope=0.2,concat=True,fill_value='mean', dropout=dropval)
        self.conv2 = GATConv(200*nheads, 100, heads=nheads,edge_dim=1,add_self_loops=self_loops,negative_slope=0.2,concat=True,fill_value='mean', dropout=dropval)
        self.conv3 = GATConv(100*nheads, 100, heads=nheads,edge_dim=1,add_self_loops=self_loops,negative_slope=0.2,concat=True,fill_value='mean')
        self.drop1 = nn.Dropout(p=dropval)
        self.GraphNorm2 = GraphNorm(100*nheads)
        self.GraphNorm1 = GraphNorm(200*nheads)
        self.layers1 = nn.Sequential(
          nn.Linear(100*nheads, 2),
          #nn.Sigmoid(),
          #nn.Linear(100, 2),
          nn.Sigmoid()
        )

    def forward(self, x, edge_index, batch):
        x = self.conv1(x, edge_index)
        #x = self.GraphNorm1(x)
        x = F.relu(x)
        x = self.conv2(x, edge_index)
        x = self.GraphNorm2(x)
        x = F.relu(x)
        x = global_mean_pool(x, batch)
        x = self.drop1(x)
        x = self.layers1(x)

        
        return x


class MCGCN(torch.nn.Module):
    def __init__(self, dropval, dataset,nheads=1,self_loops=True,graphnorm=True):
        super().__init__()
        self.conv1 = GATConv(dataset.num_node_features, 200, heads=nheads,edge_dim=1,add_self_loops=True,negative_slope=0.2,concat=True,fill_value='mean', dropout=dropval)
        self.conv2 = GATConv(200*nheads, 100, heads=nheads,edge_dim=1,add_self_loops=self_loops,negative_slope=0.2,concat=True,fill_value='mean', dropout=dropval)
        self.conv3 = GATConv(100*nheads, 100, heads=nheads,edge_dim=1,add_self_loops=self_loops,negative_slope=0.2,concat=True,fill_value='mean')
        self.drop1 = nn.Dropout(p=dropval)
        self.GraphNorm2 = GraphNorm(100*nheads)
        self.GraphNorm1 = GraphNorm(200*nheads)
        self.layers1 = nn.Sequential(
          nn.Linear(100*nheads, 100),
          nn.Sigmoid(),
          nn.Linear(100, 5),
          nn.Sigmoid()
        )

    def forward(self, x, edge_index, batch):
        x = self.conv1(x, edge_index)
        #x = self.GraphNorm1(x)
        x = F.relu(x)
        x = self.conv2(x, edge_index)
        x = self.GraphNorm2(x)
        x = F.relu(x)
        x = global_mean_pool(x, batch)
        x = self.drop1(x)
        x = self.layers1(x)


        return x



# self made grad reverse layer
class GradReverse(torch.autograd.Function):
    @staticmethod
    def forward(ctx, x):
        return x.view_as(x)

    @staticmethod
    def backward(ctx, grad_output):
        return grad_output.neg()*0.01    # the constant is to rescale the gradients from regression to classification without destabilising the classification network

def grad_reverse(x):
    return GradReverse.apply(x) 
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


def my_plot(epochs, loss, valloss, dropval,epochnr,learr,beta1,beta2,batchsize, status):
    figr, axr = plt.subplots(figsize=(15,15)) 
    axr.plot(epochs, loss, label='loss')
    axr.plot(epochs, valloss, label='validation loss')
    plt.xlabel("epochs")
    plt.ylabel("loss")
    plt.legend(loc="upper right")
    figr.savefig("/user/dmarckx/public_html/ML/NN/losses/" + status + "drop" + str(int(dropval*100)) + "_" + str(epochnr) + "_" + str(learr) + "_(" + str(int(beta1*10)) + "," + str(int(beta2*10))  + ")" + str(batchsize) + "NN2" + ".png")
    plt.close(figr) 
#==============================================================================================================#
# should work now
def TrainNN(X_train, y_train,X_test,y_test,weights_train,weights_test,classweight, dropval,learr,beta1,beta2,batchsize,status, epochs=15, manualNjobs=4):
    torch.manual_seed(42)
    torch.set_num_threads(manualNjobs)
    dataset = Dataset(X_train, y_train, weightage = weights_train)
    valset = Dataset(X_test, y_test, weightage = weights_test)
    trainloader = torch.utils.data.DataLoader(dataset, batch_size=50, shuffle=True)
    mlp = SmallNetwork(dropval)
 
    loss_function = nn.CrossEntropyLoss(reduction='none',weight=torch.tensor(classweight))
    optimizer = torch.optim.AdamW(mlp.parameters(), lr=learr)
  
    num_epochs = epochs
    loss_vals=  []
    valloss_vals = []
    # Run the training loop
    for epoch in range(0, num_epochs):
        epoch_loss= []
        epoch_valloss = []
        # print(f'Starting epoch {epoch+1}')
        for i, data in enumerate(trainloader, 0):
            inputs, targets, trainweight = data
            inputs, targets, trainweight = inputs.float(), targets.float(), trainweight.float()
            targets = targets.reshape((targets.shape[0], 2))
            optimizer.zero_grad()
            outputs = mlp(inputs)
            loss = loss_function(outputs, targets)
            loss = (trainweight * loss).mean()
            loss.backward()
            epoch_loss.append(loss.item())
            optimizer.step()
            valoutputs = mlp(valset.X.float())
            valloss = loss_function(valoutputs, valset.y.float())
            valloss = (valset.weight.float() * valloss).mean()
            epoch_valloss.append(valloss.item())
        loss_vals.append(sum(epoch_loss)/len(epoch_loss))
        valloss_vals.append(sum(epoch_valloss)/len(epoch_valloss))

    my_plot(np.linspace(1, num_epochs, num_epochs).astype(int), loss_vals, valloss_vals, dropval,epochs,learr,beta1,beta2,batchsize,status)
    return mlp

# SHOULD WORK NOW
def TrainNN2(X_train, y_train,X_test,y_test,weights_train,weights_test,classweight, dropval,learr,beta1,beta2,batchsize,status, epochs=15, manualNjobs=4):
    torch.manual_seed(42)
    torch.set_num_threads(manualNjobs)
    dataset = Dataset(X_train, y_train, weightage=weights_train)
    valset = Dataset(X_test, y_test, weightage=weights_test)
    trainloader = torch.utils.data.DataLoader(dataset, batch_size=batchsize, shuffle=True)
    mlp = SmallNetwork2(dropval)
  
    loss_function = nn.CrossEntropyLoss(reduction='none',weight=torch.tensor(classweight))
    optimizer = torch.optim.AdamW(mlp.parameters(), lr=learr, betas = (beta1,beta2), amsgrad = False) # consider using AMSgrad when the network trains longer
  
    num_epochs = epochs
    loss_vals=  []
    valloss_vals = []
    # Run the training loop
    for epoch in range(0, num_epochs):
        epoch_loss= []
        epoch_valloss = []
        print(f'Starting epoch {epoch+1}')
        for i, data in enumerate(trainloader, 0):
            inputs, targets, trainweight = data
            inputs, targets, trainweight = inputs.float(), targets.float(), trainweight.float()
            targets = targets.reshape((targets.shape[0], 2))
            optimizer.zero_grad()
            outputs = mlp(inputs)
            loss = loss_function(outputs, targets)
            loss = (trainweight * loss).mean()
            loss.backward()
            epoch_loss.append(loss.item())
            optimizer.step()
            valoutputs = mlp(valset.X.float())
            valloss = loss_function(valoutputs, valset.y.float())
            valloss = (valset.weight.float() * valloss).mean()
            epoch_valloss.append(valloss.item())
        loss_vals.append(sum(epoch_loss)/len(epoch_loss))
        valloss_vals.append(sum(epoch_valloss)/len(epoch_valloss))

    my_plot(np.linspace(1, num_epochs, num_epochs).astype(int), loss_vals, valloss_vals, dropval,epochs,learr,beta1,beta2,batchsize,status)
    return mlp

# updated but not tested
def TrainNNreg(X_train, y_train,X_test,y_test, weights_train,weights_test,classweight,epochs=15):
    torch.manual_seed(42)
    dataset = Dataset(X_train, y_train,regression=True, weightage = weight_train)
    valset = Dataset(X_test, y_test,regression=True, weightage = weight_test)
    trainloader = torch.utils.data.DataLoader(dataset, batch_size=50, shuffle=True, num_workers=2)
    mlp = SmallNetwork3()
  
    loss_function = nn.L1Loss(reduction='none')# when there is regression imbalance, it might be best to use Focal-R or (square root)inverse-frequency weighting((SQ)INV) or  Bagging-based ensemble
    optimizer = torch.optim.AdamW(mlp.parameters(), lr=0.05)
  
    num_epochs = epochs
    loss_vals=  []
    valloss_vals= []
    #Run the training loop
    for epoch in range(0, num_epochs):
        epoch_loss= []
        epoch_valloss= []

        for i, data in enumerate(trainloader, 0):
            inputs, targets, trainweight = data
            inputs, targets, trainweight  = inputs.float(), targets.float(), trainweight.float()
            targets = targets.reshape((targets.shape[0], 1))
            optimizer.zero_grad()
            outputs = mlp(inputs)
            loss = loss_function(outputs, targets)
            loss = (trainweight * loss).mean()
            loss.backward()
            epoch_loss.append(loss.item())
            optimizer.step()
            valoutputs = mlp(valset.X.float())
            valloss = loss_function(valoutputs, valset.y.float().reshape((valset.y.shape[0], 1)))      
            valloss = (valset.weight.float() * valloss).mean()
            epoch_valloss.append(valloss.item())
        loss_vals.append(sum(epoch_loss)/len(epoch_loss))
        valloss_vals.append(sum(epoch_valloss)/len(epoch_valloss))

    my_plot(np.linspace(1, num_epochs, num_epochs).astype(int), loss_vals, valloss_vals, dropval,epochs,learr,beta1,beta2,batchsize)
    return mlp

#==============================================================================================================#
# OLD
def TrainNN_hot(mlp, X_train, y_train,X_test,y_test, classweight, epochs=1):
    torch.manual_seed(42)
    dataset = Dataset(X_train, y_train)
    valset = Dataset(X_test, y_test)
    trainloader = torch.utils.data.DataLoader(dataset, batch_size=50, shuffle=True, num_workers=1)  
  
    loss_function = nn.CrossEntropyLoss(weight=torch.tensor(classweight))
    optimizer = torch.optim.AdamW(mlp.parameters(), lr=0.0001)
  
    num_epochs = epochs
    # Run the training loop
    for epoch in range(0, num_epochs):
        epoch_loss= []
        epoch_valloss = []
        for i, data in enumerate(trainloader, 0):
            inputs, targets = data
            inputs, targets = inputs.float(), targets.float()
            targets = targets.reshape((targets.shape[0], 2))
            optimizer.zero_grad()
            outputs = mlp(inputs)
            loss = loss_function(outputs, targets)
            loss.backward()
            optimizer.step()
            
    return mlp
# OLD
def TrainANN_hot(mlp, X_train, y_train, epochs=1):
    torch.manual_seed(42)
    dataset = Dataset(X_train, y_train,regression=True)
    trainloader = torch.utils.data.DataLoader(dataset, batch_size=50, shuffle=True, num_workers=1)
    
    loss_function = nn.L1Loss()
    optimizer = torch.optim.AdamW(mlp.parameters(), lr=0.05)
  
    num_epochs = epochs

    for epoch in range(0, num_epochs):
        epoch_loss= []
        epoch_valloss= []
        for i, data in enumerate(trainloader, 0):
            inputs, targets = data
            inputs, targets = inputs.float(), targets.float()
            targets = targets.reshape((targets.shape[0], 1))
            optimizer.zero_grad()
            outputs = mlp(inputs)
            loss = loss_function(outputs, targets)
            loss.backward()
            optimizer.step()

    return mlp





#==============================================================================================================#
# OLD
def TrainANN(X_train, y_train,X_test,y_test,yreg_train,yreg_test,classweight, epochs=15, lda=0.1):
    #load hot models
    torch.manual_seed(42)
    #pretrained regressor
    ANN = SmallNetwork3()
    ANN.load_state_dict(torch.load('/user/dmarckx/CMSSW_10_6_28/src/Configuration/GenProduction/trained_ANN.sav'))
    #pretrained classifier
    mlp = SmallNetwork()
    mlp.load_state_dict(torch.load('/user/dmarckx/CMSSW_10_6_28/src/Configuration/GenProduction/trained_bilin.sav'))

    
    #datasets
    dataset = Dataset(X_train, y_train,y_reg=yreg_train)
    valset = Dataset(X_test, y_test,y_reg=yreg_test)
    trainloader = torch.utils.data.DataLoader(dataset, batch_size=50, shuffle=True, num_workers=1)

    #classifier loss function
    loss_function = nn.CrossEntropyLoss(weight=torch.tensor(classweight)) 
    #ANN loss function
    loss_function2 = nn.L1Loss()
    #both use same optimizer but different learning rate
    optimizer = torch.optim.AdamW(mlp.parameters(), lr=0.0005)
    optimizer_reg = torch.optim.AdamW(mlp.parameters(), lr=0.005)

    
    #make save lists
    num_epochs = epochs
    loss_vals=  []
    valloss_vals = []
    
    # Run number of epochs
    for epoch in range(0, num_epochs):
        epoch_loss= []
        epoch_valloss = []
        
        #run number of batches
        for i, data in enumerate(trainloader, 0):
            inputs, targets, regtargets = data
            inputs, targets, regtargets = inputs.float(), targets.float(), regtargets.float()
            targets = targets.reshape((targets.shape[0], 2))
            
            #reset optimizers
            optimizer_reg.zero_grad()
            optimizer.zero_grad()

            outputs = mlp(inputs)
            
            #outputs are fed to ANN
            ANN_in = outputs
            #optimise ANN to new outputs
            ANN = TrainANN_hot(ANN, ANN_in, regtargets, epochs=1)
            ANNoutputs = ANN(ANN_in)
            loss_ANN = loss_function2(ANNoutputs, regtargets)
            loss_ANN.backward()
            optimizer_reg.step()
            optimizer_reg.zero_grad()
            ANN_in.requires_grad = True
            ANNoutputs = ANN(ANN_in)
            loss_ANN = loss_function2(ANNoutputs, regtargets)
            
            #loss is calculated and complete model is retrained
            loss = loss_function(outputs, targets) - lda* loss_ANN
            loss.backward()
            epoch_loss.append(loss.item())
            optimizer.step()
            valoutputs = mlp(valset.X.float())
            valloss = loss_function(valoutputs, valset.y.float())
            epoch_valloss.append(valloss.item())
        #save losses to plot
        loss_vals.append(sum(epoch_loss)/len(epoch_loss))
        valloss_vals.append(sum(epoch_valloss)/len(epoch_valloss))

    my_plot(np.linspace(1, num_epochs, num_epochs).astype(int), loss_vals, valloss_vals, dropval,epochs,learr,beta1,beta2,batchsize)
    return mlp


# OLD
def TrainANN2(X_train, y_train,X_test,y_test,yreg_train,yreg_test,weightage_train,weightage_test, classweight, epochs=15, lda=0.1):
    torch.manual_seed(42)
    #datasets
    dataset = Dataset(X_train, y_train,y_reg=yreg_train,weightage=weightage_train)
    valset = Dataset(X_test, y_test,y_reg=yreg_test,weightage=weightage_test)
    trainloader = torch.utils.data.DataLoader(dataset, batch_size=50, shuffle=True, num_workers=1)
    mlp = BigNetwork()
  
    loss_fn = nn.L1Loss(reduction='none')#nn.CrossEntropyLoss(weight=torch.tensor(weights))
    def loss_function(y, y_hat, w):
        return (loss_fn(y, y_hat)*w).mean()
    loss_function2 = nn.CrossEntropyLoss(weight=torch.tensor(classweight))
    optimizer = torch.optim.AdamW(mlp.parameters(), lr=0.0001)
  
    num_epochs = epochs
    loss_vals=  []
    valloss_vals = []
    class_loss_vals=  []
    class_valloss_vals = []
    # Run the training loop
    for epoch in range(0, num_epochs):
        epoch_loss= []
        epoch_valloss = []
        class_epoch_loss= []
        class_epoch_valloss = []
        # print(f'Starting epoch {epoch+1}')
        for i, data in enumerate(trainloader, 0):
            inputs, targets, regtargets,weights = data
            inputs, targets, regtargets,weights = inputs.float(), targets.float(), regtargets.float(),weights.float()
            targets = targets.reshape((targets.shape[0], 2))
            regtargets = regtargets.reshape((regtargets.shape[0], 1))
            weights = weights.reshape((weights.shape[0], 1))
            optimizer.zero_grad()
            outputs,regoutputs = mlp(inputs)
            loss = loss_function(regoutputs, regtargets,weights) + 0.05*loss_function2(outputs, targets)
            loss.backward()
            epoch_loss.append(loss.item())
            optimizer.step()
            
            classloss = loss_function2(outputs, targets)
            class_epoch_loss.append(classloss.item())
            
            valoutputs,valregoutputs = mlp(valset.X.float())
            valtargets = valset.y.float()
            valtargets = valtargets.reshape((valtargets.shape[0], 2))
            valregtargets = valset.y_reg.float()
            valregtargets = valregtargets.reshape((valregtargets.shape[0], 1))
            valweights = valset.weight.float()
            valweights = valweights.reshape((valweights.shape[0], 1))
            
            valloss = loss_function(valregoutputs, valregtargets,valweights) + 0.05*loss_function2(valoutputs,valtargets)
            epoch_valloss.append(valloss.item())
            
            classvalloss = loss_function2(valoutputs,valtargets)
            class_epoch_valloss.append(classvalloss.item())
            
        loss_vals.append(sum(epoch_loss)/len(epoch_loss))
        valloss_vals.append(sum(epoch_valloss)/len(epoch_valloss))
        
        class_loss_vals.append(sum(class_epoch_loss)/len(class_epoch_loss))
        class_valloss_vals.append(sum(class_epoch_valloss)/len(class_epoch_valloss))

    my_plot(np.linspace(1, num_epochs, num_epochs).astype(int), loss_vals, valloss_vals, dropval,epochs,learr,beta1,beta2,batchsize)
    my_plot(np.linspace(1, num_epochs, num_epochs).astype(int), class_loss_vals, class_valloss_vals, dropval,epochs,learr,beta1,beta2,"classonly")

    return mlp


def trainGCN(traindata,testdata,classweight, dropval,learr,beta1,beta2,batchsize,status,sparse='',nheads=1,self_loops=True, epochs=15, manualNjobs=4):
    torch.manual_seed(42)
    torch.set_num_threads(manualNjobs)
    
    trainloader = DataLoader(traindata, batch_size=batchsize, shuffle=True)
    testloader = DataLoader(testdata, batch_size=batchsize, shuffle=False)
    mlp = GCN(dropval, traindata[0],nheads,self_loops)
    loss_function = nn.CrossEntropyLoss(reduction='none',weight=torch.tensor(classweight))
    optimizer = torch.optim.AdamW(mlp.parameters(), lr=learr)

    num_epochs = epochs
    loss_vals=  []
    valloss_vals = []
    # Run the training loop
    for epoch in range(0, num_epochs):
        epoch_loss= []
        epoch_valloss = []
        print(f'Starting epoch {epoch+1}')
        for i, data in enumerate(trainloader, 0):
            targets, trainweight = data.y.float(), data.w.float()
            newouts = []
            for value in targets:
                if value == 1:
                    newouts.append([0, 1])
                elif value == 0:
                    newouts.append([1, 0])
            targets = torch.from_numpy(np.array(newouts)).float() 
            targets = targets.reshape((targets.shape[0], 2))
            optimizer.zero_grad()
            outputs = mlp(data.x, data.edge_index, data.batch)
            loss = loss_function(outputs, targets)
            loss = (trainweight * loss).mean()
            loss.backward()
            epoch_loss.append(loss.item())
            optimizer.step()
            valloss = 0
            for testdat in testloader:
                newtouts = []
                for value in testdat.y.float():
                    if value == 1:
                        newtouts.append([0, 1])
                    else:
                        newtouts.append([1, 0])
                valoutputs = mlp(testdat.x, testdat.edge_index, testdat.batch)
                valtargets = torch.from_numpy(np.array(newtouts)).float()
                valtargets = valtargets.reshape((valtargets.shape[0], 2))
                vallosst = loss_function(valoutputs, valtargets)
                valloss += (testdat.w.float() * vallosst).mean()
            epoch_valloss.append(valloss.item()/len(testloader))
        loss_vals.append(sum(epoch_loss)/len(epoch_loss))
        valloss_vals.append(sum(epoch_valloss)/len(epoch_valloss))
    status = status + sparse
    my_plot(np.linspace(1, num_epochs, num_epochs).astype(int), loss_vals, valloss_vals, dropval,epochs,learr,beta1,beta2,batchsize,status)
    return mlp

def trainMCGCN(traindata,testdata,classweight, dropval,learr,beta1,beta2,batchsize,status,sparse='',nheads=1,self_loops=True, epochs=15, manualNjobs=4):
    torch.manual_seed(42)
    torch.set_num_threads(manualNjobs)

    trainloader = DataLoader(traindata, batch_size=batchsize, shuffle=True)
    testloader = DataLoader(testdata, batch_size=batchsize, shuffle=False)
    mlp = MCGCN(dropval, traindata[0],nheads,self_loops)
    loss_function = nn.CrossEntropyLoss(reduction='none',weight=torch.tensor(classweight))
    optimizer = torch.optim.AdamW(mlp.parameters(), lr=learr)

    num_epochs = epochs
    loss_vals=  []
    valloss_vals = []
    # Run the training loop
    for epoch in range(0, num_epochs):
        epoch_loss= []
        epoch_valloss = []
        print(f'Starting epoch {epoch+1}')
        for i, data in enumerate(trainloader, 0):
            targets, trainweight = data.y.float(), data.w.float()
            newouts = []
            for value in targets:
                if value == 0:
                    newouts.append([1, 0, 0, 0, 0])
                elif value == 1:
                    newouts.append([0, 1, 0, 0, 0])
                elif value == 2:
                    newouts.append([0, 0, 1, 0, 0])
                elif value == 3:
                    newouts.append([0, 0, 0, 1, 0])
                elif value == 4:
                    newouts.append([0, 0, 0, 0, 1])
            targets = torch.from_numpy(np.array(newouts)).float()
            #print(targets)
            
            #targets = targets.reshape((targets.shape[0], 5))
            optimizer.zero_grad()
            outputs = mlp(data.x, data.edge_index, data.batch)

            #print(outputs) 
            loss = loss_function(outputs, targets)
            loss = (trainweight * loss).mean()
            loss.backward()
            epoch_loss.append(loss.item())
            optimizer.step()
        valloss = 0
        for testdat in testloader:
            newtouts = []
            for value in testdat.y.float():
                if value == 0:
                    newtouts.append([1, 0, 0, 0, 0])
                elif value == 1:
                    newtouts.append([0, 1, 0, 0, 0])
                elif value == 2:
                    newtouts.append([0, 0, 1, 0, 0])
                elif value == 3:
                    newtouts.append([0, 0, 0, 1, 0])
                elif value == 4:
                    newtouts.append([0, 0, 0, 0, 1])
            valoutputs = mlp(testdat.x, testdat.edge_index, testdat.batch)
            valtargets = torch.from_numpy(np.array(newtouts)).float()
            #valtargets = valtargets.reshape((valtargets.shape[0], 2))
            vallosst = loss_function(valoutputs, valtargets)
            valloss += (testdat.w.float() * vallosst).mean()
            epoch_valloss.append(valloss.item()/len(testloader))
        loss_vals.append(sum(epoch_loss)/len(epoch_loss))
        valloss_vals.append(sum(epoch_valloss)/len(epoch_valloss))
    status = status + sparse
    my_plot(np.linspace(1, num_epochs, num_epochs).astype(int), loss_vals, valloss_vals, dropval,epochs,learr,beta1,beta2,batchsize,status)
    return mlp
