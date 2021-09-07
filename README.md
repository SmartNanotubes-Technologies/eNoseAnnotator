# eNoseAnnotator

eNoseAnnotator is a program used to view, annotate and store measurements of the eNose sensor. Currently Windows systems are supported.

[![CI: Windows Build](https://img.shields.io/appveyor/ci/tilagiho/eNoseAnnotator/master?label=CI%20Windows%20Build)](https://ci.appveyor.com/project/tilagiho/eNoseAnnotator/branch/master) 


## How to install

First, download and install the [sensor drivers](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers). 
Windows 10: "CP210x Universal Windows Driver", Windows 8/ 7: "CP210x Windows Drivers"

Then download the eNoseAnnotator windows archive from the assets of the latest release (green label).
Unpack the zip file where you want to use the program. Install the vc redistributable package by running "vc_redist.x64.exe" in the directory "bin".
Finally, double-click "eNoseAnnotator.exe" in the directory "bin" to run the program.



## How to use

### Connecting a sensor

Plug in an eNose sensor. In the tool bar click the USB connection symbol, select the usb-port of the sensor and hit apply. 

To start the measurement press start symbol in the tool bar. If you have not set the functionalisation of the sensor, you will be promted to do so. After calculating the base vector of the sensor the measurement will be started. The base vector is the average vector of the first few measurements.

You can pause and stop the measurement. After stopping you can start a new measurement by pressing the start icon again. If an error occurs during a measurement, you can click the reconnect symbol in order to resume the measurement. 

### Navigation
You drag and drop the graphs to move around and use the mousewheel to zoom into and out of the graphs. Additionally you can zoom into a specific area by holding <kbd>Shift</kbd> and drawing a rectangle of the area with the <kbd>Left Mouse Button</kbd>.
Use <kbd>Shift</kbd>+<kbd>Right Mouse Button</kbd> to zoom out of the graph to show all data of the current measurement. With <kbd>Shift</kbd>+<kbd>Middle Mouse Button</kbd> you can zoom to the previous zoom area specified.

### Annotating data

The graph in the top left shows the the relative deviation of the measurements to the base vector. You can changing the range of the x-axis of the graph by dragging & dropping the graph left and right. It is also possible to zoom in and out using the mouse wheel. 

You can select data by pressing <kbd>Ctrl</kbd> and drawing a selection rectangle in the graph. After selecting data the graph on the bottom left will show the average vector of the selection. 

In order to annotate the selected data use the annotation symbol in the toolbar.

### Loading/ Saving data

You can load and save data using the the symbols in the toolbar at the top.

The data is stored in the format of a .csv file.

## Crash reports

The eNoseAnnotator automatically creates a crash report when it terminates unexpectedly. If you experience crashes, the related crash reports can help us fixing the problem.
They are stored in the temporary directory in "eNoseAnnotator/crash_reports".
### Windows:
Press <kbd>⊞ Win</kbd>+<kbd>R</kbd>, type "%TMP%" and press ok to open the local temp directory.

### Linux:
The crash reports are stored in /tmp or if the environment variable is set in TMPDIR.

## Classifiers

The vectors of a measurement can be classified using [TorchScript(.pt)](https://pytorch.org/tutorials/advanced/cpp_export.html) models. 

### Training own classfiers

You can train your own classifier using pyTorch and convert it into TorchScript. The following information should be considered when doing so.

The output of the classifier can either be a vector of the class logits or class probabilities. In order to interpret the model in the right way, some variables have to be part the model, while others are optional.

| Variable        | Optional |Type             | Description                                                                                                | Default value |
| --------------- | :------: | :-------------: | ---------------------------------------------------------------------------------------------------------- | ------------- |
| classList       |          | list of strings | list of the class names, has to be in the same order as the output vector                                  | -             |
| name            | x        | string          | name of the classifier                                                                                     | -             |
| N               | x        | integer         | number of inputs of the classifier                                                                         | -             |
| M               | x        | integer         | number of outputs of the classifier                                                                        | -             |
| input_function  | x        | string          | function applied before the input ("average", "median_average" or "None")                                  | "average"     |
| output_function | x        | string          | function applied to the output ("logsoftmax", "sigmoid" or "None")                                         | "logsoftmax"  |
| is_multi_label  | x        | bool            | If true, threshold is applied to obtain multi-label annotations. Otherwise the most likely class is picked | false         |
| threshold       | x        | double          | threshold applied to the results of the output function                                                    | 0.3           |
| isInputAbsolute | x        | bool            | true if absolute vectors should be used, otherwise relative vectors are used as input                      | false         |
| mean            | x        | list of doubles | mean for each input, should be set if the classifier's training set was normalised                         | -             |
| variance        | x        | list of doubles | variance for each input, should be set if the classifier's training set was normalised                     | -             |
| preset_name     | x        | string          | name of the sensor's functionalisation preset                                                              | -             |


