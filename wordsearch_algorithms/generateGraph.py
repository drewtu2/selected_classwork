import matplotlib.pyplot as plt
import regex

class cubeEntry:
    def __init__(self, gridSize, listSize, sortMechanism, sortTime, searchTime, totalTime):
        self.gridSize = gridSize
        self.listSize = listSize
        self.sortMechanism = sortMechanism
        self.sortTime = sortTime
        self.searchTime = searchTime
        self.totalTime = totalTime

def buildDataCube():
    timingFile = "timingData.txt"
    pattern = r"grid: data/input(?P<gridSize>\d+)?.txt\s+"
    pattern += "wordList: data/(?P<dataList>\w+)?.txt\s+"
    pattern += "sortMechanism: (?P<sortMechanism>\d+)?\s+"
    pattern += ".+Sort Time: (?P<sortTime>\d+.\d+|\d+.\d+e-\d+)?\s+"
    pattern += "Search Time: (?P<searchTime>\d+.\d+)?\s+"
    pattern += "Total Time: (?P<totalTime>\d+.\d+)?\s+"

    results = [];

    with open(timingFile) as f:
        content = f.readlines()
        for line in content:
            try:
                lineResults = regex.match(pattern, line)

                myEntry = cubeEntry(
                    lineResults.group("gridSize"),
                    lineResults.group("dataList"),
                    lineResults.group("sortMechanism"),
                    lineResults.group("sortTime"),
                    lineResults.group("searchTime"),
                    lineResults.group("totalTime"))
                
                results.append(myEntry)
            except AttributeError as e:
                print("Error: ", e)
    print("Generate dataCube with " + str(len(results)) + " timing results")

    return results

def mech2str(mechanism):
    mechanism = int(mechanism)
    if mechanism == 1:
        return "Insert"
    elif mechanism == 2:
        return "Quick"
    elif mechanism == 3:
        return "Merge"
    elif mechanism == 4:
        return "STL"
    elif mechanism == 5:
        return "Hash"
    elif mechanism == 6:
        return "Heap"
    else:
        return "Error..."

def generateGraph():
    results = buildDataCube();
    x_labels = []
    x = []
    y_totalTime = []
    y_searchTime = []
    y_sortTime = []

    for index, entry in enumerate(results):
        if int(entry.sortMechanism) != 1:
             x.append(int(index) + 1)
             x_labels.append(str(entry.gridSize) + "\n" + mech2str(entry.sortMechanism))
             y_totalTime.append(float(entry.totalTime))
             y_searchTime.append(float(entry.searchTime))
             y_sortTime.append(float(entry.sortTime))
  
    plt.title("Overall Run Time of Search Algorithm")
    for index in range(1, int(len(results)/3)):
        plt.subplot(2,3,index)
        sortPlot = plt.bar(x[3*index:3*(index + 1)], y_sortTime[3*index:3*(index + 1)])
        searchPlot = plt.bar(x[3*index:3*(index + 1)], y_searchTime[3*index:3*(index + 1)], bottom = y_sortTime[3*index:3*(index + 1)])
        plt.xticks(x[3*index:3*(index + 1)], x_labels[3*index:3*(index + 1)])

    plt.xticks(x, x_labels)
    
    plt.ylabel("Time (s)")
    plt.xlabel("Run")
    plt.show()


generateGraph()
#buildDataCube()
