#include "../include/R&D.h"
#include "../include/Accountant.h"
#include "../include/Project.h"
#include <algorithm>
#include <random>

ResearchAndDevelopment::ResearchAndDevelopment(std::vector<Worker>& workers, double approvalThreshold)
    : workers(workers), approvalThreshold(approvalThreshold), nextIdeaId(1) {
}

int ResearchAndDevelopment::submitIdea(Worker& worker, const std::string& ideaDescription) {
    ResearchIdea idea;
    idea.id = nextIdeaId++;
    idea.description = ideaDescription;
    idea.submitter = &worker;
    idea.status = IdeaStatus::SUBMITTED;
    idea.researchProgress = 0;
    idea.requiredResearchTime = estimateResearchTime(idea);
    idea.feasibilityScore = 0.0;
    idea.rejectionReason = "";
    
    submittedIdeas.push_back(idea);
    return idea.id;
}

void ResearchAndDevelopment::processSubmittedIdeas() {
    for (auto& idea : submittedIdeas) {
        if (idea.status == IdeaStatus::SUBMITTED) {
            idea.status = IdeaStatus::RESEARCHING;
            conductResearch(idea.id);
        }
    }
}

void ResearchAndDevelopment::conductResearch(int ideaId) {
    auto* idea = getIdeaById(ideaId);
    if (idea && idea->status == IdeaStatus::RESEARCHING) {
        // Simulate research progress
        idea->researchProgress += 10; // 10% progress per research cycle
        
        if (idea->researchProgress >= idea->requiredResearchTime) {
            idea->feasibilityScore = calculateFeasibilityScore(*idea);
            
            if (evaluateIdea(ideaId)) {
                approveIdea(ideaId);
            } else {
                rejectIdea(ideaId, "Insufficient feasibility score or resources");
            }
        }
    }
}

void ResearchAndDevelopment::research() {
    // Legacy method - conducts research on all active ideas
    for (auto& idea : submittedIdeas) {
        if (idea.status == IdeaStatus::RESEARCHING) {
            conductResearch(idea.id);
        }
    }
}

bool ResearchAndDevelopment::evaluateIdea(int ideaId) {
    auto* idea = getIdeaById(ideaId);
    if (!idea) return false;
    
    return idea->feasibilityScore >= approvalThreshold && hasRequiredExpertise(*idea);
}

void ResearchAndDevelopment::approveIdea(int ideaId) {
    auto* idea = getIdeaById(ideaId);
    if (idea) {
        idea->status = IdeaStatus::APPROVED;
        
        // Move to approved ideas list
        auto it = std::find_if(submittedIdeas.begin(), submittedIdeas.end(),
            [ideaId](const ResearchIdea& i) { return i.id == ideaId; });
        if (it != submittedIdeas.end()) {
            approvedIdeas.push_back(*it);
        }
    }
}

void ResearchAndDevelopment::rejectIdea(int ideaId, const std::string& reason) {
    auto* idea = getIdeaById(ideaId);
    if (idea) {
        idea->status = IdeaStatus::REJECTED;
        idea->rejectionReason = reason;
    }
}

void ResearchAndDevelopment::approve() {
    // Legacy method - approves first viable idea
    for (auto& idea : submittedIdeas) {
        if (idea.status == IdeaStatus::RESEARCHING && evaluateIdea(idea.id)) {
            approveIdea(idea.id);
            break;
        }
    }
}

void ResearchAndDevelopment::sendApprovedIdeasToAccountant(Accountant& accountant) {
    for (auto& idea : approvedIdeas) {
        if (idea.status == IdeaStatus::APPROVED) {
            Project* project = createProjectFromIdea(idea);
            if (project) {
                accountant.receiveProjectFromRD(*this, project, idea.description);
                idea.status = IdeaStatus::SENT_TO_ACCOUNTANT;
            }
        }
    }
}

Project* ResearchAndDevelopment::createProjectFromIdea(const ResearchIdea& idea) {
    // Create a project based on the research idea
    // This is a simplified implementation
    int projectId = idea.id;
    int estimatedHours = static_cast<int>(idea.feasibilityScore * 200); // Convert feasibility to hours
    int workersNeeded = std::max(1, estimatedHours / 40); // Assuming 40 hours per worker
    
    return new Project(projectId, estimatedHours, workersNeeded);
}

ResearchIdea* ResearchAndDevelopment::getIdeaById(int id) {
    // Search in submitted ideas first
    auto it = std::find_if(submittedIdeas.begin(), submittedIdeas.end(),
        [id](const ResearchIdea& i) { return i.id == id; });
    if (it != submittedIdeas.end()) {
        return &(*it);
    }
    
    // Search in approved ideas
    auto it2 = std::find_if(approvedIdeas.begin(), approvedIdeas.end(),
        [id](const ResearchIdea& i) { return i.id == id; });
    if (it2 != approvedIdeas.end()) {
        return &(*it2);
    }
    
    return nullptr;
}

int ResearchAndDevelopment::getActiveResearchCount() const {
    return std::count_if(submittedIdeas.begin(), submittedIdeas.end(),
        [](const ResearchIdea& i) { return i.status == IdeaStatus::RESEARCHING; });
}

double ResearchAndDevelopment::calculateFeasibilityScore(const ResearchIdea& idea) {
    // Simplified feasibility calculation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    double baseScore = dis(gen);
    
    // Adjust based on description length (simple heuristic)
    double descriptionFactor = std::min(1.0, idea.description.length() / 100.0);
    
    // Adjust based on submitter's experience (simplified)
    double submitterFactor = idea.submitter ? 
        std::min(1.0, idea.submitter->getLaborTimeHoursWorked() / 1000.0) : 0.5;
    
    return (baseScore * 0.5) + (descriptionFactor * 0.3) + (submitterFactor * 0.2);
}

bool ResearchAndDevelopment::hasRequiredExpertise(const ResearchIdea& idea) {
    // Check if we have workers with sufficient experience
    int experiencedWorkers = 0;
    for (const Worker& worker : workers) {
        if (worker.getLaborTimeHoursWorked() > 500) { // Arbitrary threshold
            experiencedWorkers++;
        }
    }
    
    return experiencedWorkers >= 2; // Need at least 2 experienced workers
}

int ResearchAndDevelopment::estimateResearchTime(const ResearchIdea& idea) {
    // Simple estimation based on idea complexity
    int baseTime = 50; // Base research time
    int complexityFactor = static_cast<int>(idea.description.length() / 10);
    
    return baseTime + complexityFactor;
} 