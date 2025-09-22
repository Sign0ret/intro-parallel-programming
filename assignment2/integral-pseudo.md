# Solving an integral through approximation using paraleling programming.

1. Get the number of threads, and number of trapezoids.
2. Get the x_step calculated by 1 / number of trapezoids.
3. Get the distributed_trapezoids = amount of trapezoids each thread should do by trapezoids / threads.
4. Start the n threads passing them the start point, the amount of distributed_trapezoids, and the value of the x_step.
5. In the thread function start a variable to store the sum of the areas.
6. Calculate each area from 0-distributed_trapezoids using the corresponding x_step and the x_step.
    7. Add the returned value to the variable.
    8. Make the start point to become start point + x_step.
9. When the whole thread finishes, return the final variable value.
10. Use a mutex to add the values of each thread to a final variable.
11. Return the final value of this variable.