# Adaptive Traffic Control System

This project implements a real-time adaptive traffic light control system on an STM32 microcontroller using FreeRTOS. The firmware is structured as a modular, event-driven embedded system in which each intersection operates as an independent RTOS task. Traffic signal timing is dynamically adjusted based on real-time traffic conditions, enabling congestion-aware phase control and scalable multi-intersection management. Inter-task communication is handled using FreeRTOS message queues and event flags to ensure deterministic and responsive system behavior.

The system is complemented by a host-side Python-based user interface developed with Qt. This interface provides real-time visualization and interaction with the embedded system via UART communication. Traffic conditions and control commands are injected at runtime, enabling dynamic simulation of congestion scenarios without firmware recompilation. The interface visualizes intersection states, signal phases, and congestion levels, allowing direct observation of the system’s adaptive behavior under varying traffic loads.

## Generate Code from STM32CubeMX

- Multiple STM32CubeMX `.ioc` files are stored in the `MXCube/` directory. Each file corresponds to a specific STM32F411 board variant (e.g., Nucleo, Discovery).

- To generate code from a specific `.ioc` configuration:

1. Copy the selected `.ioc` file into the project root directory.  
2. Rename the `.ioc` file to match the exact project root folder name (required for STM32CubeIDE project recognition).  
3. Ensure that pin assignments and peripheral configurations are compatible across board variants when shared behavior is expected.  
4. After code generation, verify that changes in auto-generated files correspond to expected `.ioc` modifications. Any mismatch may indicate configuration drift between versions.  
5. After modification, copy the updated `.ioc` file back into the `MXCube/` directory following the predefined naming convention. The `.ioc` file in the project root should be treated as a temporary working file and is excluded from version control.
