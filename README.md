
## Running the algorithm

* Enter the Program directory: `cd Program`
* Run the make command: `make test`
* Try another example: `./icavrp -i X-n157-k13`

The following options are supported:
```
Usage:
  ./icavrp [-it nbIter] [-t timeLimit] [-s mySeed] [-veh nbVehicles] [-p InitialCountries] [-ii initialImp] [-rr RevolRate]
Available options:
  -it           Sets a maximum number of iterations without improvement. Defaults to 20,000
  -t            Sets a time limit in seconds. If this parameter is set, the code will be restart iteratively until the time limit
  -s            Sets a fixed seed. Defaults to 1
  -veh          Sets a prescribed fleet size. Otherwise a reasonable UB on the fleet size is calculated
  -p            Sets initial countries
  -ii           Sets the number of initial imperialists, default is 3
  -rr           Sets the revolution rate, default is 0.5
```
