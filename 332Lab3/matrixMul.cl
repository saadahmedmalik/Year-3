__kernel void matrixMul(
    __global const float* input_title,
    __global const float* weights_title,
    const int input_title_size,
    const int output_neurons_title_size,
    __global float* output_title
)

{
    // Each work item computes one element of the output vector
    int neuron_id = get_global_id(0);

    if (neuron_id < output_neurons_title_size) {
       
        // The input vector is 16 elements long, and the weights are stored in a 2D array
        // so instead of doing a the loop im unrolling and adding the sums 16 times
        float sum = 0.0f;
        int offset = neuron_id * input_title_size;
        sum += input_title[0] * weights_title[offset];
        sum += input_title[1] * weights_title[offset + 1];
        sum += input_title[2] * weights_title[offset + 2];
        sum += input_title[3] * weights_title[offset + 3];
        sum += input_title[4] * weights_title[offset + 4];
        sum += input_title[5] * weights_title[offset + 5];
        sum += input_title[6] * weights_title[offset + 6];
        sum += input_title[7] * weights_title[offset + 7];
        sum += input_title[8] * weights_title[offset + 8];
        sum += input_title[9] * weights_title[offset + 9];
        sum += input_title[10] * weights_title[offset + 10];
        sum += input_title[11] * weights_title[offset + 11];
        sum += input_title[12] * weights_title[offset + 12];
        sum += input_title[13] * weights_title[offset + 13];
        sum += input_title[14] * weights_title[offset + 14];
        sum += input_title[15] * weights_title[offset + 15];
        
        // Store the result in the output vector
        output_title[neuron_id] = sum;
    }
}