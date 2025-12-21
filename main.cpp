#include "Distribution.hpp"
#include <iostream>
#include <iomanip>
#include <sys/stat.h>
#include <chrono>

static void	createOutputDir(const std::string &path)
{
	mkdir(path.c_str(), 0755);
}

static void	printModeStats(const Distribution &dist, const std::string &mode)
{
	std::cout << "  " << mode << ": "
			  << dist.simulationCount(mode) << " sims, RTP "
			  << std::fixed << std::setprecision(2)
			  << (dist.getRTP(mode) * 100.0) << "%" << std::endl;
}

int	main(void)
{
	Distribution	dist;
	std::string		outputDir;
	size_t			numSimulations;

	outputDir = "output";
	numSimulations = 100000;
	createOutputDir(outputDir);

	// === MODE BASE (cost 1.0) ===
	dist.addMode("base", 1.0);
	dist.addMultiplier("base", 0.0, 350);
	dist.addMultiplier("base", 0.5, 250);
	dist.addMultiplier("base", 1.0, 200);
	dist.addMultiplier("base", 1.5, 120);
	dist.addMultiplier("base", 2.0, 80);

	// === MODE BONUS (cost 100.0) - meilleur RTP ===
	dist.addMode("bonus", 100.0);
	dist.addMultiplier("bonus", 0.0, 100);
	dist.addMultiplier("bonus", 1.0, 200);
	dist.addMultiplier("bonus", 2.0, 300);
	dist.addMultiplier("bonus", 5.0, 250);
	dist.addMultiplier("bonus", 10.0, 100);
	dist.addMultiplier("bonus", 50.0, 40);
	dist.addMultiplier("bonus", 100.0, 10);

	// Lance les simulations
	std::cout << "Running " << numSimulations << " simulations per mode..."
			  << std::endl;
	auto start = std::chrono::high_resolution_clock::now();
	dist.runSimulations("base", numSimulations, 42);
	dist.runSimulations("bonus", numSimulations, 123);
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
		(end - start);
	std::cout << "Done in " << duration.count() << "ms" << std::endl;
	std::cout << std::endl;

	// Stats
	std::cout << "=== Results ===" << std::endl;
	printModeStats(dist, "base");
	printModeStats(dist, "bonus");
	std::cout << std::endl;

	// Export
	std::cout << "Exporting files..." << std::endl;
	dist.exportAll(outputDir);

	return (0);
}
