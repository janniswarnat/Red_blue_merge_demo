# Computing a single face in an arrangement of line segments

## Abstract

In the context of a diploma thesis at the university of Bonn we present an implementation of the deterministic algorithm for constructing a single face in an arrangement of line segments using CGAL´s arrangement package and its sweep line framework. The algorithm is developed and presented in the book “Davenport-Schinzel sequences and their geometric applications" by Micha Sharir and Pankaj K. Agarwal, Cambridge University Press, 1995.

While the complexity of a full arrangement of line segments is in O(n2), Sharir and Agarwal show that any single face in such arrangement has a maximum complexity of O(α(n)*n) where α(n) denotes the extremely slow-growing inverse Ackermann function which can be regarded as constant for any conceivable real-world input. Any single face can be constructed in time O(α(n)*n*log2n) using a deterministic divide and conquer algorithm including – in the words of Sharir and Agarwal – a “sophisticated sweep-line technique” in the merge step ("red blue merge").

## Run on Windows

Install VcXsrv Windows X Server (https://sourceforge.net/projects/vcxsrv/)

Configure and save configuration (see https://dev.to/darksmile92/run-gui-app-in-linux-docker-container-on-windows-host-4kde)

```bash
docker-compose up
```

