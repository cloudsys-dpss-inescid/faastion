## Manager configuration

### Description

Before each operation with the lambda manager, we need to specify the manager's behavior using this configuration. The
proper description of each field in this configuration is provided below.

Default value for each variable rest inside configs/manager/default-lambda-manager.json.

```json
{
  "gateway": "[STRING] The default PC's gateway address with mask?",
  "maxMemory": "[INTEGER] Maximum memory available for all lambdas (in MBs)",
  "maxTaps": "[INTEGER] Maximum number of taps created for lambdas",
  "timeout": "[INTEGER] Time during which lambda can stay inactive?",
  "healthCheck": "[INTEGER] Lambda's health will be checked in this time-span, after the first health response, no more checks are made.",
  "memory": "[STRING] Maximum memory consumption per active lambda?",
  "lambdaPort": "[INTEGER] In which port the lambda will receive it's requests?",
  "lambdaConsole": "[BOOL] Is console active during qemu's run?",
  "managerConsole": {
    "turnOff": "[BOOL] Turn On/Off logging",
    "redirectToFile": "[BOOL] Should the logging be redirected to file or printed in console?",
    "fineGrain": "[BOOL] Fine or coarse grain logging?"
  },
  "managerState": {
    "scheduler": "[STRING] Fully qualified name of chosen Scheduler?",
    "encoder": "[STRING] Fully qualified name of chosen Encoder?",
    "storage": "[STRING] Fully qualified name of chosen Storage?",
    "client": "[STRING] Fully qualified name of chosen Client?",
    "codeWriter": "[STRING] Fully qualified name of chosen Code writer?",
    "lambdaInfo": {
      "lambda": "[STRING] Fully qualified name of chosen Lambda?",
      "function": "[STRING] Fully qualified name of chosen Function?"
    }
  }
}
```

---

## Installing dependencies

### General

The tool for installing dependencies `install-deps.py` will install all tools and prepare an environment for further
testing/developing.

### Arguments

```commandline
run install-deps
run install-deps verbosity_level
```

The tool receives non or one argument, if no arguments have been passed the default verbosity level will be set to 0, if
the argument has been passed then it should be in the format *v* or *vv* (verbosity level 1 or 2). <br/>
Verbosity levels:

- Level 0 - only mandatory information will be printed.
- Level 1 - level 0 plus error and warning logs.
- Level 2 - level 1 plus full output log.

---

## Build

### General

The tool for building components of the system (all at once or separately) sits inside `run_build.py`.

### Arguments

```commandline
run build
run build comma-separate-list-components
```

Calling this command without arguments will build a load balancer, cluster manager, lambda manager, and lambda proxy all
at once. The second argument is used for selecting only particular component of system (load balancer [lb], cluster
manager [cm], lambda manager [lm] or lambda proxy [lp]).

## Deploy

### General

The tool for deploying components of the system (all at once or separately) sits inside `run_deploy.py`.

### Arguments

```commandline
run deploy
run deploy comma-separate-list-components
```

Calling this command without arguments will deploy a load balancer, cluster manager, lambda manager, and lambda proxy
all at once. The second argument is used for selecting only particular component of system (load balancer [lb], cluster
manager [cm], lambda manager [lm] or lambda proxy [lp]).

## Testing

### General

The tool for running tests `run_test.py` will create multi-user, multi-client (multiple clients behind the same
username) an environment with different loads based on test configuration for system stress testing.

There is also possibility to start all test at once, using  `run_tests.py`. Test are separate into different tiers,
based on level of complexity.

### Arguments

#### Separate testing

```commandline
run test test_config_path
run test verbosity_level test_config_path
```

The tool receives one or two-argument, if one argument has been passed the default verbosity level will be set to 0, if
two arguments have been passed then the first one should be in the format *v* or *vv* (verbosity level 1 or 2). <br/>
Verbosity levels:

- Level 0 - only mandatory information will be printed.
- Level 1 - level 0 plus error and warning logs.
- Level 2 - level 1 plus full output log.

The second argument of the tool is a path for the test configuration. A detailed explanation of the configuration
structure is provide bellow (value inside [ ] represents JSON data types):

```json
{
  "test": "[STRING] Test name?",
  "entry_point": "[STRING] Load balancer address?",
  "managers": [
    {
      "address": "[STRING] 1th lambda manager address?",
      "config_path": "[STRING] 1th lambda manager configuration path?",
      "register_in_load_balancer": "[BOOL] Should we register this manager into load balancer?"
    }
  ],
  "users": [
    {
      "username": "[STRING] 1th username?",
      "failure_pattern": "[STRING] Pattern is detected in lambda manage log in case of test failure.",
      "commands": [
        {
          "command": "remove",
          "function_name": "[STRING] Function name?"
        },
        {
          "command": "upload",
          "allocate": "[INTEGER] How many instances the user wants allocate for this function?",
          "function_name": "[STRING] Function name?",
          "source": "[STRING] Function source code path (root directory)?"
        },
        {
          "command": "send",
          "sending_info": [
            {
              "iterations": "[INTEGER] How many test iterations...",
              "num_requests": "[INTEGER] with how many request per iteration...",
              "num_clients": "[INTEGER] with how many clients in parallel?",
              "function_name": "[STRING] Function name?",
              "parameters": {
                "param1_name": "[ANY] param1",
                "param2_name": "[ANY] param2",
                "param3_name": "[ANY] param3"
              },
              "output": "[STRING] Where the ApacheBench should store results?"
            }
          ]
        },
        {
          "command": "pause",
          "duration": "[INTEGER] How many seconds to wait?"
        }
      ]
    }
  ]
}
```

#### Test all at once (gates)

```commandline
run tests
run tests verbosity_level
run tests verbosity_level comma-sep-filter-list
```

The tool receives none, one or two argument. If there are no arguments, the default verbosity level will be set to 0, if
the argument has been passed then it should be in the format *v* or *vv* (verbosity level 1 or 2). Verbosity level will
be sent to each test separately. Second argument is comma separated list of filers. Filter can be tier number (tier-1,
tier-2...) or language (java, python...), in case when user wants to specify only a subset of tests.<br/>

---

## Plotting

### General

The tool for generating plots `run_plot.py` will output plots based on plot configuration.

### Arguments

```commandline
run plot test_config_path
run plot verbosity_level test_config_path
```

The tool receives one or two-argument, if one argument has been passed the default verbosity level will be set to 0, if
two arguments have been passed then the first one should be in the format *v* or *vv* (verbosity level 1 or 2). <br/>
Verbosity levels:

- Level 0 - only mandatory information will be printed.
- Level 1 - level 0 plus error and warning logs.
- Level 2 - level 1 plus full output log.

The second argument of the tool is a path for the plot configuration. A detailed explanation of the configuration
structure is provided bellow (value inside [ ] represents JSON data types):

```json
{
  "plot": "[STRING] Plotting name?",
  "start_server": {
    "port": "[INTEGER] Server port?",
    "img_width": "[INTEGER] Image width?",
    "img_height": "[INTEGER] Image height?"
  },
  "plots": [
    {
      "type": "latency",
      "output": "[STRING] Where to store plot image?",
      "show_image": "[BOOL] Show image after it's generated?",
      "data": [
        {
          "title": "[STRING] 1th title?",
          "filepath": "[STRING] Path to 1th input datafile?",
          "regex": "[STRING] Regex for above datafile?"
        },
        {
          "title": "[STRING] 2th title?",
          "filepath": "[STRING] Path to 2th input datafile?",
          "regex": "[STRING] Regex for above datafile?"
        },
        {
          "title": "[STRING] 3th title?",
          "filepath": "[STRING] Path to 3th input datafile?",
          "regex": "[STRING] Regex for above datafile?"
        }
      ]
    },
    {
      "type": "throughput",
      "output": "[STRING] Where to store plot image?",
      "show_image": "[BOOL] Show image after it's generated?",
      "data": [
        {
          "title": "[STRING] 1th title?",
          "filepath": "[STRING] Path to 1th input datafile?",
          "regex": "[STRING] Regex for above datafile?"
        },
        {
          "title": "[STRING] 2th title?",
          "filepath": "[STRING] Path to 2th input datafile?",
          "regex": "[STRING] Regex for above datafile?"
        },
        {
          "title": "[STRING] 3th title?",
          "filepath": "[STRING] Path to 3th input datafile?",
          "regex": "[STRING] Regex for above datafile?"
        }
      ]
    },
    {
      "type": "footprint",
      "output": "[STRING] Where to store plot image?",
      "show_image": "[BOOL] Show image after it's generated?",
      "data": [
        {
          "title": "Lambda footprint",
          "regex": "Maximum resident set size \\(kbytes\\):\\s*(\\d*)"
        }
      ]
    },
    {
      "type": "startup",
      "output": "[STRING] Where to store plot image?",
      "show_image": "[BOOL] Show image after it's generated?",
      "data": [
        {
          "title": "Lambda startup",
          "regex": "[STRING] Regex for startup?"
        }
      ]
    },
    {
      "type": "scalability",
      "output": "[STRING] Where to store plot image?",
      "show_image": "[BOOL] Show image after it's generated?",
      "data": [
        {
          "title": "System scalability"
        }
      ]
    },
    {
      "type": "total memory",
      "output": "[STRING] Where to store plot image?",
      "show_image": "[BOOL] Show image after it's generated?",
      "data": [
        {
          "title": "Total system memory",
          "regex": "[STRING] Regex for total system memory?"
        }
      ]
    },
    {
      "type": "cdf",
      "output": "[STRING] Where to store plot image?",
      "show_image": "[BOOL] Show image after it's generated?",
      "data": [
        {
          "title": "CDF"
        }
      ]
    }
  ]
}
```

---

## Monitoring

### General

The tool for collecting request latency and startup time metrics `run_measurements.py` will fetch values from manager
logs and send them to the database for further processing and plotting.

### Arguments

```commandline
run measurements config_path
```

The argument of the tool is a path for the measurement configuration. A detailed explanation of the configuration
structure is provided bellow (value inside [ ] represents JSON data types):

```json
{
  "influxdb_url": "[STRING] URL of the InfluxDB instance",
  "influxdb_token": "[STRING] Authentication token for the InfluxDB instance",
  "influxdb_org": "[STRING] Name of the organization in InfluxDB",
  "influxdb_bucket": "[STRING] Name of the bucket that stores metrics data",
  "interval": "[INTEGER] Time in seconds to sleep between reading and parsing manager logs"
}
```
