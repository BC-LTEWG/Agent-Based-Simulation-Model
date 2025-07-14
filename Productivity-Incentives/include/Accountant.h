#pragma once
#include "Firm.h"
#include "Distributor.h"
#include "Project.h"
#include "Worker.h"
#include <vector>
#include <map>
#include <chrono>

class ResearchAndDevelopment;

enum class ProjectStatus {
    RECEIVED_FROM_RD,
    APPROVED,
    REJECTED,
    SENT_TO_FIRM,
    IN_PROGRESS,
    COMPLETED
};

struct ProjectProposal {
    int id;
    std::string description;
    Project* project;
    ProjectStatus status;
    double estimatedLaborTime;
    double actualLaborTime;
    double estimatedCost;
    double actualCost;
    std::string rejectionReason;
    std::chrono::system_clock::time_point submissionDate;
    std::chrono::system_clock::time_point completionDate;
};

struct ProductivityMetrics {
    std::string projectType;
    double averageEstimatedTime;
    double averageActualTime;
    double productivityImprovement;
    int timesCompleted;
    std::vector<double> historicalTimes;
};

class Accountant {
    public:
        Accountant(Firm& f, Distributor& d, double approvalBudget = 10000.0);

        // Project management from R&D
        void receiveProjectFromRD(ResearchAndDevelopment& rd, Project* project, const std::string& description);
        void approveProject(int projectId);
        void rejectProject(int projectId, const std::string& reason);
        void sendApprovedProjectsToFirm();
        
        // Labor time calculations
        void calculateHoursForProject(Project& p);
        double calculateProjectLaborTime(const Project& project);
        double adjustLaborTimeForProductivity(const std::string& projectType, double baseTime);
        
        // Productivity tracking and quarterly reviews
        void recordProjectCompletion(int projectId, double actualLaborTime, double actualCost);
        void conductQuarterlyReview();
        void updateProductivityMetrics(const std::string& projectType, double actualTime, double estimatedTime);
        
        // Historical data analysis
        ProductivityMetrics* getProductivityMetrics(const std::string& projectType);
        double getAverageProductivityImprovement(const std::string& projectType);
        void analyzeProductivityTrends();
        
        // Budget and cost management
        bool canApproveProject(double estimatedCost);
        void updateBudget(double amount);
        double calculateProjectCost(const Project& project);
        
        // Legacy labor time exchange tracking
        void trackLaborTimeExchange(Worker& worker, Firm& firm, int laborTimeHours);
        void trackGoodsExchange(Firm& firm, Distributor& distributor, const std::vector<GoodsService>& goods);
        
        // Reporting and analytics
        void generateLaborTimeReport();
        void generateProductivityReport();
        void generateQuarterlyReport();
        double calculateTotalLaborTimeValue(const std::vector<Worker*>& workers);
        
        // Getters
        Firm& getFirm() { return firm; }
        Distributor& getDistributor() { return distributor; }
        const std::vector<ProjectProposal>& getProjectProposals() const { return projectProposals; }
        double getAvailableBudget() const { return availableBudget; }
        int getApprovedProjectCount() const;
        int getRejectedProjectCount() const;

    private:
        Firm& firm;
        Distributor& distributor;
        double availableBudget;
        double totalLaborTimeTracked;
        double totalValueTracked;
        
        // Project tracking
        std::vector<ProjectProposal> projectProposals;
        std::map<std::string, ProductivityMetrics> productivityData;
        int nextProjectId;
        
        // Quarterly review data
        std::chrono::system_clock::time_point lastQuarterlyReview;
        
        // Internal calculation methods
        double calculateBaseEstimate(const Project& project);
        std::string categorizeProject(const Project& project);
        bool isWithinBudget(double cost);
        void updateHistoricalData(const std::string& projectType, double actualTime);
};
