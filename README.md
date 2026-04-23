    ~PATH-FINDER
USING ASTAR ALGORITHM

Pathfinder: Interactive Pathfinding VisualizerPathfinder is an interactive desktop application built in C that visualizes the A pathfinding algorithm* in real-time. Developed as a Foundation of Programming mini-project, it serves as an educational tool to help students understand the dynamic, step-by-step nature of algorithm execution through hands-on exploration.


🚀 FeaturesReal-Time A Visualization:* Watch the algorithm explore the grid, manage open/closed lists, and construct the optimal path with smooth animations.

*Recursive Maze Generation*: Instantly generate "perfect" mazes (randomized, no loops, exactly one solution) using a recursive backtracking algorithm.Interactive Grid 


*Editing*: Click and drag to place walls, move start (green) and end (red) nodes, or clear the grid to test custom 
scenarios.Optimized 

*Logic*: Guarantees the shortest path using Euclidean distance heuristics.


🛠️ Tech Stack
Language: C.


🏗️ Project ArchitectureThe codebase follows a modular header-based organization to ensure maintainability:

*Node.h*: Defines the node structure, including g, h, and f costs and parent pointers

*.State.h*: Manages node states (EMPTY, WALL, START, END, OPEN, CLOSED, PATH) using enumerations.Grid.h: Handles the 2D grid management, user interactions, and maze generation logic

*.AStar.h*: Contains the core pathfinding logic, including neighbor exploration and cost updates.Main.h: The entry point handling SDL2 initialization and the main application loop.


🧪 Methodology: A* LogicThe algorithm evaluates nodes based on the following cost functions:gCost: The actual distance from the starting node to the current node.hCost (Heuristic):
The estimated distance from the current node to the target using Euclidean distance.fCost: The sum of gCost and hCost ($f = g + h$). The algorithm always explores the node with the lowest fCost next.


👥 Team Members
Sharvani Walawalkar (1262251167)

Rashi Malpani (1262252822)

Akshara Dubey (1262253416) 

Devesh Patil (1262251847)

Neer Agrawal (1262252059) 


Mentor: Dr. Uma Pujeri 
Academic Session: 2025-26 
