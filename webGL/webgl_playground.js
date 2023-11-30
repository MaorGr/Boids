function parseCSV(text) {
    // Split the text into lines (each line represents an entity)
    const lines = text.trim().split('\n').slice(1);
  
    // Map each line into a 2D array of [x, y] coordinates for each timepoint
    return lines.map(line =>
      line.split(',').slice(1).reduce((coords, value, index) => {
        const coordinateIndex = Math.floor(index / 2);
        if (!coords[coordinateIndex]) {
          coords[coordinateIndex] = [];
        }
        coords[coordinateIndex].push(Number(value));
        return coords;
      }, [])
    );
  }

  async function loadAndParseCSV(url) {
    const response = await fetch(url);
    const text = await response.text();
    return parseCSV(text);
  }
  
function log3DArrayDimensions(arr) {
    const dim1 = arr.length;  // Number of entities
    const dim2 = arr.every(subArr => Array.isArray(subArr)) ? Math.max(...arr.map(subArr => subArr.length)) : 0; // Max length of coordinate arrays
    const dim3 = arr.length > 0 && arr[0].length > 0 && Array.isArray(arr[0][0]) ? arr[0][0].length : 0; // Length of individual coordinate array
    
    console.log(`Dimensions of the 3D array: [${dim1}][${dim2}][${dim3}]`);
    }

function flattenDataForWebGL(data) {
    let vertices = [];
    console.log(data)
    data.forEach(entity => {
        entity.forEach(timepoint => {
            // Assuming each 'timepoint' is an array [x, y]
            vertices.push(...timepoint);
        });
    });
    return new Float32Array(vertices);
}

function flattenTimePointDataForWebGL(data) {
    let vertices = [];
    data.forEach(entity => {
        vertices.push(...entity);
    });
    return new Float32Array(vertices);
}

function compileShader(gl, shaderSource, shaderType) {
    const shader = gl.createShader(shaderType);
    gl.shaderSource(shader, shaderSource);
    gl.compileShader(shader);

    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
        console.error('An error occurred compiling the shaders: ' + gl.getShaderInfoLog(shader));
        gl.deleteShader(shader);
        return null;
    }
    console.log('Shader compliled');
    return shader;
}

function initWebGLWithData(data) {
    const flattenedData = flattenDataForWebGL(data);
    const vertices = new Float32Array(flattenedData);

    // Create a buffer object and set it as the active buffer
    const vertexBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuffer);
    const isVertexBufferBound = gl.getParameter(gl.ARRAY_BUFFER_BINDING) === vertexBuffer;
    console.log('Is vertex buffer bound:', isVertexBufferBound);

    // // Pass the vertex data to the buffer
    console.log('Transformed vertex data (10 items):', vertices.slice(0, 10));
    gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);

    // Create and compile the vertex shader
    vertexShader = compileShader(gl, vertexShaderSource, gl.VERTEX_SHADER);
    // Create and compile the fragment shader
    fragmentShader = compileShader(gl, fragmentShaderSource, gl.FRAGMENT_SHADER);
    // fragmentShader = compileShader(gl, gaussianShaderSource, gl.FRAGMENT_SHADER);
    
    

    // Create a shader program and attach the vertex and fragment shaders
    const shaderProgram = gl.createProgram();
    gl.attachShader(shaderProgram, vertexShader);
    gl.attachShader(shaderProgram, fragmentShader);

    // Link the program and use it
    gl.linkProgram(shaderProgram);

    if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
        console.error('Shader program linking error:', gl.getProgramInfoLog(shaderProgram));
    }
    
    const resolutionUniformLocation = gl.getUniformLocation(shaderProgram, "u_resolution");
    gl.useProgram(shaderProgram);
    gl.uniform2f(resolutionUniformLocation, canvas.width, canvas.height);

    // Get the attribute location, enable it
    const coord = gl.getAttribLocation(shaderProgram, 'coordinates');
    gl.vertexAttribPointer(coord, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(coord);

    // console.log(">>")
    // console.log(coord)

    // Store the necessary info for rendering
    window.vertexBuffer = vertexBuffer;
    window.shaderProgram = shaderProgram;
    window.numberOfVertices = vertices.length / 2;
}

function render_() {
    // Clear the canvas
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    // Use the shader program
    gl.useProgram(window.shaderProgram);

    // Bind the buffer
    gl.bindBuffer(gl.ARRAY_BUFFER, window.vertexBuffer);

    // Draw the points or lines
    gl.drawArrays(gl.LINES, 0, window.numberOfVertices);

    // Call render again on the next frame
    requestAnimationFrame(render);
}

function render() {
    updateWebGLDataForTimepoint(data, currentTimepoint);
    
    gl.useProgram(window.shaderProgram);
    
    // Clear and draw as before
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);
    
    
    const pointSizeLocation = gl.getUniformLocation(shaderProgram, "u_PointSize");
    const pointSize = 10.0; // Example point size
    gl.enable(gl.BLEND);
    gl.blendFunc(gl.ONE, gl.ONE);
    gl.uniform1f(pointSizeLocation, pointSize);
    
    gl.drawArrays(gl.POINTS, 0, window.numberOfVertices);
    gl.disable(gl.BLEND);
    // Update the timepoint for the next frame
    currentTimepoint = (currentTimepoint + 1) % maxTimepoint;

    // Repeat the render function at the next frame
    setTimeout(() => requestAnimationFrame(render), timeInterval);
}

function transformData(data, maxValueX, maxValueY) {
    return data.map((value, index) => {
        // Check if the index is even or odd
        if (index % 2 === 0) {
            // Even index, scale by maxValueX
            // console.log(value, " , ", (value / maxValueX) * 2 - 1)
            return (value / maxValueX) * 2 - 1;
        } else {
            // Odd index, scale by maxValueY
            return (value / maxValueY) * 2 - 1;
        }
    });
}

function getDataForTimepoint(data, timepoint) {
    return data[timepoint]; // Returns the 'entity' array for the given timepoint
}

function updateWebGLDataForTimepoint(data, timepoint) {
    const timepointData = getDataForTimepoint(data, timepoint);
    const flattenedData = flattenTimePointDataForWebGL(timepointData);
    
    const transformedData = transformData(flattenedData, 1200, 1200)
    const vertices = new Float32Array(transformedData);

    gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);
    window.numberOfVertices = vertices.length / 2;
}

// Vertex Shader GLSL code
const vertexShaderSource = `
attribute vec2 coordinates;
uniform float u_PointSize;
void main(void) {
    gl_Position = vec4(coordinates, 0.0, 1.0);
    gl_PointSize = u_PointSize;
}`;

// Fragment Shader GLSL code
const fragmentShaderSource = `
void main(void) {
    gl_FragColor = vec4(0.1, 0.1, 0.1, 1.0);  // White color
}`;

const gaussianShaderSource = `
precision mediump float;

uniform sampler2D u_texture;
uniform float u_resolution;
varying vec2 v_texCoord;

void main() {
    float kernel[9];
    kernel[0] = 1.0/16.0; kernel[1] = 1.0/8.0; kernel[2] = 1.0/16.0;
    kernel[3] = 1.0/8.0;  kernel[4] = 1.0/4.0; kernel[5] = 1.0/8.0;
    kernel[6] = 1.0/16.0; kernel[7] = 1.0/8.0; kernel[8] = 1.0/16.0;

    vec2 onePixel = vec2(1.0, 1.0) / u_resolution;
    
    vec4 colorSum =
        texture2D(u_texture, v_texCoord + onePixel * vec2(-1, -1)) * kernel[0] +
        texture2D(u_texture, v_texCoord + onePixel * vec2( 0, -1)) * kernel[1] +
        texture2D(u_texture, v_texCoord + onePixel * vec2( 1, -1)) * kernel[2] +
        texture2D(u_texture, v_texCoord + onePixel * vec2(-1,  0)) * kernel[3] +
        texture2D(u_texture, v_texCoord + onePixel * vec2( 0,  0)) * kernel[4] +
        texture2D(u_texture, v_texCoord + onePixel * vec2( 1,  0)) * kernel[5] +
        texture2D(u_texture, v_texCoord + onePixel * vec2(-1,  1)) * kernel[6] +
        texture2D(u_texture, v_texCoord + onePixel * vec2( 0,  1)) * kernel[7] +
        texture2D(u_texture, v_texCoord + onePixel * vec2( 1,  1)) * kernel[8];

    gl_FragColor = colorSum;
}`

// const densityfragmentShaderSource = `
// precision mediump float;
// uniform vec2 u_points[N]; // Array of point positions
// uniform float u_radius; // Radius for density calculation
// void main() {
//     float density = 0.0;
//     for (int i = 0; i < N; i++) {
//         if (distance(gl_FragCoord.xy, u_points[i]) < u_radius) {
//             density += 1.0;
//         }
//     }
//     // Map density to a color
//     gl_FragColor = vec4(density, density, density, 1.0);
// }`;

// Get the canvas element
var canvas = document.getElementById('webglCanvas');
canvas.width = 1200;
canvas.height = 1200;
const gl = canvas.getContext('webgl');
if (!gl) {
    console.error('WebGL is not supported by your browser.');
}

let data; // The 3D array with your data
let currentTimepoint = 0;
let maxTimepoint; 
const timeInterval = 100;

async function main() {
    try {
        const csvUrl = 'http://localhost:8080/boid_paths_STAR00.csv';
        data = await loadAndParseCSV(csvUrl);
        currentTimepoint = 0;
        maxTimepoint = data.length;
        const timeInterval = 100; // Time per timepoint in milliseconds

        log3DArrayDimensions(data);

        initWebGLWithData(data);

        // Start the rendering loop
        render();
    } catch (error) {
        console.error("Error in the application:", error);
    }
}

main();
