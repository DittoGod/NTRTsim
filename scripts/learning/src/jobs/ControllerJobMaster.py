import logging
import json
import random

from algorithms import dispatchLearning
from helpersNew import Generation
from helpersNew import Controller
from helpersNew import dictTools
from concurrent_scheduler import ConcurrentScheduler
from LearningJobMaster import LearningJobMaster
#TODO: This is hackety, fix it.
from evolution import EvolutionJob

class ControllerJobMaster(LearningJobMaster):


    """
    Component = the actual dictionary
    Component Name = the string name of the component

    For MonteCarlo, generationSize is king for how many members are made
    You can theoretically have a smaller generation than pop size for each MC
    """

    NEURAL_NET_DIRECTORY_NAME = "NeuralNet/"

    def _setup(self):

        logging.getLogger("NTRT Learning")
        super(ControllerJobMaster, self)._setup()

        # Create directory for NN files
        neuralNetDirectory = self.trialDirectory + '/' + self.NEURAL_NET_DIRECTORY_NAME
        dictTools.tryMakeDir(neuralNetDirectory)

    def generationGenerator(self, previousGeneration):
        components = self.getComponents()
        generationID = self.getNewGenerationID(previousGeneration)

        logging.info("Starting algorithms")
        componentPopulations = self.generateComponentPopulations(components, previousGeneration, generationID)
        # Perform generation creation
        newGeneration = self.createNewGeneration(componentPopulations, generationID)

        return newGeneration

    def generateComponentPopulations(self, componentDictionary, previousGeneration, generationID):
        #print "genID in generateComponentPopulations: " + str(generationID)
        componentPopulations = {}
        for componentName in componentDictionary:
            population = self.generateComponentPopulation(componentName,
                                                          componentDictionary,
                                                          previousGeneration,
                                                          generationID)
            componentPopulations[componentName] = population
        return componentPopulations

    # "Generation" is expliclty the generation object
    # "Population" is just a list or dictionary
    def generateComponentPopulation(self, componentName, components, previousGeneration, generationID):
        #print "genID in generateComponentPopulation: " + str(generationID)
        componentDictionary = components[componentName]
        emptyComponent = self.createEmptyComponent(componentName, components[componentName]['NeuralNetwork'], generationID)
        componentPopulation = dispatchLearning(componentName=componentName,
                                            componentDictionary=componentDictionary,
                                            templateComponent=emptyComponent,
                                            previousGeneration=previousGeneration
                                            )
        ### Make component backwards compatible
        ### Move this to its own function when possible
        ### Metrics is not added because it is never seen to be used
        for component in componentPopulation:
            # Index the component in the line of all of these components made so far in learning
            populationSize = self.config[componentName]['PopulationSize']
            component['paramID'] = populationSize * generationID + component['populationID']
            component['scores'] = []

            ### Hacky hacky hacky
            ### HACKY HACKY HACKY
            if "numHidden" in component['params']:
                baseFileName = self.config['PathInfo']['fileName'] + "_" + componentName + "_" + str(component['generationID']) + "_" + str(component['populationID']) + '.nnw'
                fileName = self.trialDirectory + '/' + self.NEURAL_NET_DIRECTORY_NAME + baseFileName
                self.writeToNNW(component['params']['neuralParams'], fileName)
                component['params']['neuralFilename'] = self.NEURAL_NET_DIRECTORY_NAME + baseFileName

        return componentPopulation

    """
    This is taken straight from Brian's code.
    Clean up later before major commit
    """
    def writeToNNW(self, neuralParams, fileName):
        fout = open(fileName, 'w')
        first = True
        for x in neuralParams:
            if (first):
                fout.write(str(x))
                first = False
            else:
                fout.write("," + str(x))

    def createEmptyComponent(self, component, neuralNet, generationID):
        emptyComponent = None
        # print "Current component: " + component
        if neuralNet['numberOfStates'] == 0:
            emptyComponent = self.getNonNNParams(neuralNet)
        else:
            emptyComponent = self.getNNParams(neuralNet)
        # dictTools.printDict(emptyComponent)
        emptyComponent['generationID'] = generationID
        return emptyComponent

    def getNonNNParams(self, neuralNet):
        numInstances = neuralNet['numberOfInstances']
        numOutputs = neuralNet['numberOfOutputs']
        params = []

        for i in range(numInstances):
            subParams = []
            for j in range(numOutputs):
                subParams.append(None)
            params.append(subParams)

        return {'params' : params}

    def getNNParams(self, neuralNet):
        numOutputs = neuralNet['numberOfOutputs']
        numStates = neuralNet['numberOfStates']
        numHidden = neuralNet['numberOfHidden']
        totalParams = (numStates + 1) * (numHidden) + (numHidden + 1) * numOutputs

        newNeuro = {
            'neuralParams' : [None] * totalParams,
            'numActions'   : numOutputs,
            'numStates'    : numStates,
            'numHidden'    : numHidden
            #'neuralFilename' : "logs/bestParameters-test_fb-"+ newController['paramID'] +".nnw"}
        }
        return {'params' : newNeuro}

    # Need to set up a call system to this from child jobs
    # Maybe pass it a function of type generation -> generation?
    def beginTrial(self):
        self.beginTrialMaster(generationGeneratorFuction=self.generationGenerator)

"""
    def temp(self):
        for keys in lParams:
            if keys[-4:] == "Vals":
                self.prefixes.append(keys[:-4])

        print (self.prefixes)

        self.currentGeneration = {}
        for p in self.prefixes:
            self.currentGeneration[p] = {}

        for n in range(generationCount):
            # Create the generation'
            for p in self.prefixes:
                self.currentGeneration[p] = self.generationGenerator(self.currentGeneration[p], p + 'Vals')

        return super(ControllerJobMaster, self).generationGenerator()
"""
