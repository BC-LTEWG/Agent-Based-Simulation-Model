#pragma once
#include <vector>
#include <string>
#include <memory>
#include "Worker.h"

class Accountant;
class Project;

enum class IdeaStatus {
    SUBMITTED,
    RESEARCHING,
    APPROVED,
    REJECTED,
    SENT_TO_ACCOUNTANT
};

struct ResearchIdea {
    int id;
    std::string description;
    Worker* submitter;
    IdeaStatus status;
    int researchProgress;
    int requiredResearchTime;
    double feasibilityScore;
    std::string rejectionReason;
};

class ResearchAndDevelopment {
    public:
        ResearchAndDevelopment(std::vector<Worker>& workers, double approvalThreshold = 0.7);

        // Idea submission and management
        int submitIdea(Worker& worker, const std::string& ideaDescription);
        void processSubmittedIdeas();
        
        // Research process
        void conductResearch(int ideaId);
        void research(); // Legacy method - conducts research on all active ideas
        
        // Approval/rejection process
        bool evaluateIdea(int ideaId);
        void approveIdea(int ideaId);
        void rejectIdea(int ideaId, const std::string& reason);
        void approve(); // Legacy method - approves first viable idea
        
        // Integration with Accountant
        void sendApprovedIdeasToAccountant(Accountant& accountant);
        Project* createProjectFromIdea(const ResearchIdea& idea);
        
        // Getters and status
        const std::vector<ResearchIdea>& getSubmittedIdeas() const { return submittedIdeas; }
        const std::vector<ResearchIdea>& getApprovedIdeas() const { return approvedIdeas; }
        ResearchIdea* getIdeaById(int id);
        int getTotalIdeasSubmitted() const { return nextIdeaId - 1; }
        int getActiveResearchCount() const;

    private:
        std::vector<Worker>& workers;
        std::vector<ResearchIdea> submittedIdeas;
        std::vector<ResearchIdea> approvedIdeas;
        double approvalThreshold;
        int nextIdeaId;
        
        // Research evaluation methods
        double calculateFeasibilityScore(const ResearchIdea& idea);
        bool hasRequiredExpertise(const ResearchIdea& idea);
        int estimateResearchTime(const ResearchIdea& idea);
};