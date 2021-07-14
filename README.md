SplineFitting
=============

C++ code for illustrating Spline Fitting in OpenGL

Design a tool where a user can interactively draw a sequence of control point and draw a smooth curve using the following techniques:

(i) By computing appropriate first derivatives in the parametric space and specifying end conditions (by drawing arrows for showing the direction of derivative), to get a piecewise smooth second order continuos curves passing through each of these points,

(ii) By using cubic B-Splines and Beta-splines (with appropriate parameters as input) through these control points,

(iii) By drawing n-th order Bezier curve with (n+1) control points.

 

The tool should also have the following features:

(i) Interactive dragging of a control point, causing the change in the shape of the curve. When the desried shape is attained, it should be saved with parameters, etc.

(ii) The tool can simultaneously handle different sets or groups of control point. Each group is editable with the following operations:

      (a) Deletion of a point,

      (b) Insertion of a new point, and

      (c) Modification of a point.

(iii) Save the whole confuguration in a file using your own file format.

(iv) Load a file containing groups of control points and parameters.


![Alt Text](https://github.com/PranjalSahu/SplineFitting/blob/master/spline_fitting.gif)
