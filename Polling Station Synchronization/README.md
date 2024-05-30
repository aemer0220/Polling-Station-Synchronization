# CS441/541 Synchronization Project

## Author(s):

Alexandra Emerson


## Date:

4/19/2024


## Description:

This program simulates an election polling station that allows Democrats, Republicans, and Independents to vote. Each voter goes through the entire process of waiting for the polling station to open, registering to vote, waiting for a booth, and voting. The caveat is that, while Independents can vote with any party, Democrats and Republicans cannot vote at the same time. Through the use of Pthreads and semaphores, this program synchronizes the voting while also preventing deadlock.


## How to build the software

After importing the source code into a Linux based command prompt, provide the command 
```
make
```
to utilize the given Makefile. This will compile the program.  

Providing the command
```
make clean
```
will clean up the code by removing all previously compiled object files from the source code.

## How to use the software

To run the Finicky Voter program after compiling, please provide the following command...
```
./finicky-voter [#1] [#2] [#3] [#4] 
```
where the hashtagged integers are replaced by four optional, non-negative integers. These arguments correspond to and specify specific parameters within the program, as follows:  

Integer #  | Parameter
------------- | -------------
#1   | Number of voting booths
#2   | Number of Democrats
#3   | Number of Republicans
#4   | Number of Independents

If any one of the parameters is not provided, the default values are as follows:
Parameter  | Default Value
------------------ | -------------
Number of voting booths   | 10
Number of Democrats   | 5
Number of Republicans   | 5
Number of Independents   | 5

## How the software was tested

Initial testing involved verifying the program was performing command-line parsing correctly. This involved doing runs with one, two, three, and four integer parameters to ensure the defaults were being set and replaced properly. I also ensured invalid input was caught and handled, by running the program with non-integer inputs such as symbols, characters, and negative numbers.

To test the implementation of the program, I ran it several times with various advantages/disadvantages among each of the parameters. 

For example, to test the program where Republicans had more voters than the other parties, I would run something like,
```
./finicky-voter 10 20 5 5
```
and so on. Testing the proportion between the number of voters in each political party, and the number voting booths, truly allows you to gain an understanding of how the program behaves.

## Known bugs and problem areas

There are no known bugs or problem areas.

## Special Section

### Describe your solution to the problem
* My solution utilizes a combination of the barrier and turnstile techniques to coordinate the voting process and prevent Democrats and Republicans from being able to vote with one another. The barrier technique is used to wait until all voters are waiting for the polling station to open. After it opens, another barrier is encountered that prevents voters from waiting for a booth until all voters are in the polling station. After this, the turnstile manner of voting can begin.
*  For example, if a Republican is the first to wait for a booth, all Republicans that show up to wait for a booth directly after will get to vote. If a Democrat shows up and sees that there are currently Republicans voting, they will need to wait until the last Republican currently voting leaves the booth area, which will signal the Democrats that they can stop waiting.
* Independents are, well, independent from the voting turnstile. The only time they need to wait is if the number of voters in line for a booth totals the number of booths. Otherwise, they can go right to the first available booth.  
  
### Describe how your solution meets the goals of this project
* How does your solution avoid deadlock?
    - My solution avoids deadlock through the use of sempahores. There are several semaphores, including print and count semaphores to protect the print methods and various count variables, respectively. Other semaphores to prevent deadlock include line mutexes to prevent more than one voter from getting in line at the same time, as well as semaphores for each party that tell the voters when they need to wait to get in line to vote, and when they can go to a booth.
* How does your solution prevent Republican and Democrat voters from waiting on a voting booth at the same time?
    - My solution prevents this conflict by having separate queues for them to line up in, and semaphores indicating that one party has ownership of the booth at that time and that the other party must wait. A turnstile-like technique is used so that each party votes in waves. When the last Democrat or Republican leaves the booth (depending on who has ownership), the other party is signaled to go ahead and find a booth to vote.
* How is your solution "fair"? How are you defining "fairness" for this problem"?
    - I would define fairness in that everyone gets to vote by taking turns. In my solution, a group/wave of one party will vote, and then once they all leave the voting booth, those from the other party waiting will get to vote.
    - Fairness is also implemented in that Independents, even though they can vote with either Democrats or Republicans, do not get to cut in line. If the voting group (turnstile) is full and an Independent comes in to vote, they need to wait until a spot is available.
* How does your solution prevent starvation? How are you defining "starvation" for this problem
    - Starvation is defined as one party being forever prevented from being able to vote. i.e. If one party is voting and more voters of the same party come in, voters of the opposing party would be continuously pushed to the back of the line.
    - Starvation is prevented through the turnstile technique. Voters get to vote in waves, based on their party, where the turnstile swaps between the two parties. Specifically for Independents, starvation is prevented by letting them vote with anyone, but in turn, they are unable to starve other parties with the prevention of cutting in line if the booths are full.
* How do you balance fairness with resource utilization and throughput? How would you measure this?
    - The balance between fairness, resource utilization, and throughput can be measured as a function of everyone getting through the voting process in the least amount of time/steps possible overall.
    - The turnstile technique itself is known for poor resource utilization, but my solution makes better utilization of resources, as well as manages the balance between resource utilization, throughput, and fairness, by allowing Independents in right away when there is a spot available, and also by taking turns in a FIFO matter between each of the opposing parties.
