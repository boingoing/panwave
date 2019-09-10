[![master build status](https://travis-ci.org/boingoing/panwave.svg?branch=master)](https://travis-ci.org/boingoing/panwave/builds#)

# panwave

A library for computing decomposing and reconstructing signals via wavelet packet trees.

## Why panwave

Panwave is adapted from the wavelet packet tree component of a hobby project named PAN built way back in 2008. For the tenth anniversary of the project, panwave was split out into its own library and re-implemented in c++17. Panwave is the PAN wavelet (PanWave) component.

## Using panwave

Panwave offers a simple interface for performing wavelet packet tree operations.

```c++
std::vector<double> signal = {0,1,2,3,4,5,6,7,8,9,10,11,12};
std::vector<double> reconstructed_signal;
reconstructed_signal.resize(signal.size());

// Load well-known wavelet coefficients.
Wavelet wavelet;
Wavelet::GetWaveletCoefficients(&wavelet, Wavelet::WaveletType::Daubechies, 4);

// Construct wavelet packet tree of height = 3.
// This tree has 4 wavelet levels, and 4 leaves.
WaveletPacketTree tree(3, &wavelet);
tree.SetRootSignal(&signal);
tree.Decompose();

// Reconstruct signal one wavelet level at a time.
for (size_t i = 0; i < tree.GetWaveletLevelCount(); i++) {
    tree.Reconstruct(i);
    auto root_signal = tree.GetRootSignal();
    
    // Add the coefficients of wavelet level i to the reconstructed signal.
    std::transform(reconstructed_signal.cbegin(), reconstructed_signal.cend(), root_signal->cbegin(), reconstructed_signal.begin(), std::plus<double>());
}

// signal == reconstructed_signal
```

## Building panwave

You can build panwave on any platform with a compiler which supports c++17 language standards mode. The library is designed to be portable and easy to add to your project. Add the panwave source files in `panwave/src` to your build definition and you should be ready to use panwave.

### Tested build configurations

Windows 10
* CMake 3.13.0-rc3
* Visual Studio 2017 15.8.9

Ubuntu 18.04
* CMake 3.10.2
* Clang 6.0.0

## Testing panwave

The library ships with a simple test program in the `panwave/test` folder.

```console
> git clone https://github.com/boingoing/panwave/panwave.git
> cd panwave/out
> cmake ..
> make
> ./panwave_test
```

### Using Visual Studio on Windows

Above `cmake` command generates a Visual Studio solution file (`panwave/out/panwave_test.sln`) on Windows platforms with Visual Studio. You can open this solution in Visual Studio and use it to build the test program.

## Documentation

https://boingoing.github.io/panwave/html/annotated.html
