# E-commerce System

## Project Overview

This project is an e-commerce system that consists of multiple components, including customers, suppliers, and transporters. The system uses Redis and PostgreSQL for data storage and communication. The main components of the system are:

- **Customer**: Represents a customer who can place orders for products.
- **Supplier (Fornitore)**: Represents a supplier who processes orders and updates product availability.
- **Transporter (Trasportatore)**: Represents a transporter who delivers orders to customers.

## Prerequisites

Before setting up the project, ensure you have the following software and libraries installed:

- **Redis**: An in-memory data structure store used as a database, cache, and message broker.
- **PostgreSQL**: A powerful, open-source object-relational database system.
- **C++ Compiler**: A C++17 compatible compiler (e.g., GCC).
- **Libraries**:
  - `hiredis`: A minimalistic C client for Redis.
  - `libpqxx`: The official C++ client API for PostgreSQL.

## Installation

Follow these steps to set up the project:

1. **Clone the repository**:
   ```sh
   git clone https://github.com/ToloCops/e-commerce.git
   cd e-commerce
   ```

2. **Install dependencies**:
   - Install Redis:
     ```sh
     sudo apt-get install redis-server
     ```
   - Install PostgreSQL:
     ```sh
     sudo apt-get install postgresql postgresql-contrib
     ```
   - Install C++ libraries:
     ```sh
     sudo apt-get install libhiredis-dev libpqxx-dev
     ```

3. **Build the project**:
   ```sh
   make
   ```

## Usage

To run the different components of the system, use the following commands:

1. **Run the Customer component**:
   ```sh
   ./bin/customer
   ```

2. **Run the Supplier (Fornitore) component**:
   ```sh
   ./bin/fornitore <supplier_name>
   ```
   Replace `<supplier_name>` with the name of the supplier (e.g., "apple", "samsung").

3. **Run the Transporter (Trasportatore) component**:
   ```sh
   ./bin/trasportatore
   ```

## Contributing

We welcome contributions to the project! To contribute, follow these steps:

1. **Fork the repository** on GitHub.
2. **Clone your forked repository** to your local machine:
   ```sh
   git clone https://github.com/<your-username>/e-commerce.git
   cd e-commerce
   ```
3. **Create a new branch** for your feature or bugfix:
   ```sh
   git checkout -b my-feature-branch
   ```
4. **Make your changes** and commit them with a descriptive message:
   ```sh
   git add .
   git commit -m "Add new feature"
   ```
5. **Push your changes** to your forked repository:
   ```sh
   git push origin my-feature-branch
   ```
6. **Create a pull request** on the original repository and describe your changes.

If you encounter any issues or have suggestions for improvements, please open an issue on GitHub.

Thank you for contributing!
