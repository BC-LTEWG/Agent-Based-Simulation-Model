#include "../include/Accountant.h"
#include "../include/R&D.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

Accountant::Accountant(Firm& f, Distributor& d, double approvalBudget)
    : firm(f), distributor(d), availableBudget(approvalBudget),
      totalLaborTimeTracked(0.0), totalValueTracked(0.0), nextProjectId(1) {
    lastQuarterlyReview = std::chrono::system_clock::now();
}

void Accountant::receiveProjectFromRD(ResearchAndDevelopment& rd, Project* project, const std::string& description) {
    if (project) {
        ProjectProposal proposal;
        proposal.id = nextProjectId++;
        proposal.description = description;
        proposal.project = project;
        proposal.status = ProjectStatus::RECEIVED_FROM_RD;
        proposal.estimatedLaborTime = calculateProjectLaborTime(*project);
        proposal.actualLaborTime = 0.0;
        proposal.estimatedCost = calculateProjectCost(*project);
        proposal.actualCost = 0.0;
        proposal.submissionDate = std::chrono::system_clock::now();
        
        projectProposals.push_back(proposal);
    }
}

void Accountant::approveProject(int projectId) {
    auto it = std::find_if(projectProposals.begin(), projectProposals.end(),
        [projectId](const ProjectProposal& p) { return p.id == projectId; });
    
    if (it != projectProposals.end()) {
        if (canApproveProject(it->estimatedCost)) {
            it->status = ProjectStatus::APPROVED;
            availableBudget -= it->estimatedCost;
        }
    }
}

void Accountant::rejectProject(int projectId, const std::string& reason) {
    auto it = std::find_if(projectProposals.begin(), projectProposals.end(),
        [projectId](const ProjectProposal& p) { return p.id == projectId; });
    
    if (it != projectProposals.end()) {
        it->status = ProjectStatus::REJECTED;
        it->rejectionReason = reason;
    }
}

void Accountant::sendApprovedProjectsToFirm() {
    for (auto& proposal : projectProposals) {
        if (proposal.status == ProjectStatus::APPROVED) {
            proposal.status = ProjectStatus::SENT_TO_FIRM;
            // Here you could notify the firm about the approved project
        }
    }
}

void Accountant::calculateHoursForProject(Project& p) {
    double laborTime = calculateProjectLaborTime(p);
    std::string projectType = categorizeProject(p);
    double adjustedTime = adjustLaborTimeForProductivity(projectType, laborTime);
    
    // Update project with adjusted labor time
    totalLaborTimeTracked += adjustedTime;
}

double Accountant::calculateProjectLaborTime(const Project& project) {
    return calculateBaseEstimate(project);
}

double Accountant::adjustLaborTimeForProductivity(const std::string& projectType, double baseTime) {
    auto it = productivityData.find(projectType);
    if (it != productivityData.end()) {
        double improvement = it->second.productivityImprovement;
        return baseTime * (1.0 - improvement);
    }
    return baseTime;
}

void Accountant::recordProjectCompletion(int projectId, double actualLaborTime, double actualCost) {
    auto it = std::find_if(projectProposals.begin(), projectProposals.end(),
        [projectId](const ProjectProposal& p) { return p.id == projectId; });
    
    if (it != projectProposals.end()) {
        it->status = ProjectStatus::COMPLETED;
        it->actualLaborTime = actualLaborTime;
        it->actualCost = actualCost;
        it->completionDate = std::chrono::system_clock::now();
        
        std::string projectType = categorizeProject(*it->project);
        updateProductivityMetrics(projectType, actualLaborTime, it->estimatedLaborTime);
    }
}

void Accountant::conductQuarterlyReview() {
    auto now = std::chrono::system_clock::now();
    auto timeSinceLastReview = std::chrono::duration_cast<std::chrono::hours>(now - lastQuarterlyReview);
    
    if (timeSinceLastReview.count() >= 2160) { // 90 days * 24 hours
        analyzeProductivityTrends();
        generateQuarterlyReport();
        lastQuarterlyReview = now;
    }
}

void Accountant::updateProductivityMetrics(const std::string& projectType, double actualTime, double estimatedTime) {
    auto& metrics = productivityData[projectType];
    metrics.projectType = projectType;
    metrics.historicalTimes.push_back(actualTime);
    metrics.timesCompleted++;
    
    // Calculate new averages
    double totalActual = 0.0;
    for (double time : metrics.historicalTimes) {
        totalActual += time;
    }
    metrics.averageActualTime = totalActual / metrics.historicalTimes.size();
    
    // Calculate productivity improvement
    if (estimatedTime > 0) {
        metrics.productivityImprovement = std::max(0.0, 1.0 - (metrics.averageActualTime / estimatedTime));
    }
}

ProductivityMetrics* Accountant::getProductivityMetrics(const std::string& projectType) {
    auto it = productivityData.find(projectType);
    return (it != productivityData.end()) ? &it->second : nullptr;
}

double Accountant::getAverageProductivityImprovement(const std::string& projectType) {
    auto* metrics = getProductivityMetrics(projectType);
    return metrics ? metrics->productivityImprovement : 0.0;
}

void Accountant::analyzeProductivityTrends() {
    // Analyze trends for each project type
    for (auto& [projectType, metrics] : productivityData) {
        if (metrics.historicalTimes.size() >= 2) {
            // Calculate trend based on recent vs older completion times
            size_t half = metrics.historicalTimes.size() / 2;
            double recentAvg = 0.0, olderAvg = 0.0;
            
            for (size_t i = 0; i < half; ++i) {
                olderAvg += metrics.historicalTimes[i];
            }
            olderAvg /= half;
            
            for (size_t i = half; i < metrics.historicalTimes.size(); ++i) {
                recentAvg += metrics.historicalTimes[i];
            }
            recentAvg /= (metrics.historicalTimes.size() - half);
            
            if (olderAvg > 0) {
                metrics.productivityImprovement = std::max(0.0, (olderAvg - recentAvg) / olderAvg);
            }
        }
    }
}

bool Accountant::canApproveProject(double estimatedCost) {
    return estimatedCost <= availableBudget;
}

void Accountant::updateBudget(double amount) {
    availableBudget += amount;
}

double Accountant::calculateProjectCost(const Project& project) {
    return project.getTotalLaborTimeHoursForProject() * 15.0; // Assuming $15/hour
}

void Accountant::trackLaborTimeExchange(Worker& worker, Firm& firm, int laborTimeHours) {
    totalLaborTimeTracked += laborTimeHours;
}

void Accountant::trackGoodsExchange(Firm& firm, Distributor& distributor, const std::vector<GoodsService>& goods) {
    for (const auto& good : goods) {
        totalValueTracked += good.laborTimeValue;
    }
}

void Accountant::generateLaborTimeReport() {
    std::cout << "\n=== Labor Time Report ===" << std::endl;
    std::cout << "Total Labor Time Tracked: " << totalLaborTimeTracked << " hours" << std::endl;
    std::cout << "Total Value Tracked: $" << std::fixed << std::setprecision(2) << totalValueTracked << std::endl;
    std::cout << "Available Budget: $" << availableBudget << std::endl;
}

void Accountant::generateProductivityReport() {
    std::cout << "\n=== Productivity Report ===" << std::endl;
    for (const auto& [projectType, metrics] : productivityData) {
        std::cout << "Project Type: " << projectType << std::endl;
        std::cout << "  Times Completed: " << metrics.timesCompleted << std::endl;
        std::cout << "  Average Actual Time: " << metrics.averageActualTime << " hours" << std::endl;
        std::cout << "  Productivity Improvement: " << (metrics.productivityImprovement * 100) << "%" << std::endl;
    }
}

void Accountant::generateQuarterlyReport() {
    std::cout << "\n=== Quarterly Report ===" << std::endl;
    generateLaborTimeReport();
    generateProductivityReport();
    
    std::cout << "\nProject Status Summary:" << std::endl;
    std::cout << "  Approved Projects: " << getApprovedProjectCount() << std::endl;
    std::cout << "  Rejected Projects: " << getRejectedProjectCount() << std::endl;
}

double Accountant::calculateTotalLaborTimeValue(const std::vector<Worker*>& workers) {
    double totalValue = 0.0;
    for (const Worker* worker : workers) {
        totalValue += worker->getTotalLaborTimeValue();
    }
    return totalValue;
}

int Accountant::getApprovedProjectCount() const {
    return std::count_if(projectProposals.begin(), projectProposals.end(),
        [](const ProjectProposal& p) { return p.status == ProjectStatus::APPROVED; });
}

int Accountant::getRejectedProjectCount() const {
    return std::count_if(projectProposals.begin(), projectProposals.end(),
        [](const ProjectProposal& p) { return p.status == ProjectStatus::REJECTED; });
}

double Accountant::calculateBaseEstimate(const Project& project) {
    return project.getTotalLaborTimeHoursForProject() * 1.2; // 20% buffer
}

std::string Accountant::categorizeProject(const Project& project) {
    // Simple categorization based on project size
    int totalHours = project.getTotalLaborTimeHoursForProject();
    if (totalHours < 100) return "Small";
    else if (totalHours < 500) return "Medium";
    else return "Large";
}

bool Accountant::isWithinBudget(double cost) {
    return cost <= availableBudget;
}

void Accountant::updateHistoricalData(const std::string& projectType, double actualTime) {
    productivityData[projectType].historicalTimes.push_back(actualTime);
} 