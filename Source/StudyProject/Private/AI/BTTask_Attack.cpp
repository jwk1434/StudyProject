#include "AI/BTTask_Attack.h"
#include "Controllers/SAIController.h"
#include "Characters/SNonPlayerCharacter.h"

UBTTask_Attack::UBTTask_Attack()
{
    bNotifyTick = true;
}

void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    ASAIController* AIC = Cast<ASAIController>(OwnerComp.GetAIOwner());
    if (true == ::IsValid(AIC))
    {
        ASNonPlayerCharacter* NPC = Cast<ASNonPlayerCharacter>(AIC->GetPawn());
        if (true == ::IsValid(NPC))
        {
            if (false == NPC->IsNowAttacking())
            {
                FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
            }
        }
    }
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

    ASAIController* AIC = Cast<ASAIController>(OwnerComp.GetAIOwner());
    if (true == ::IsValid(AIC))
    {
        ASNonPlayerCharacter* NPC = Cast<ASNonPlayerCharacter>(AIC->GetPawn());
        if (true == ::IsValid(NPC))
        {
            NPC->Attack();
        }
    }

    return EBTNodeResult::InProgress;

    /*공격 Task는 공격 애니메이션이 끝날 때까지 대기해야 하는 지연 Task임.
        그래서 ExecuteTask()가 InProgress를 일단 반환하고 공격이 끝났을 때
        Task가 끝났다고 알려줘야함.
        이를 알려주는 함수가 FinishLatentTask() 함수.
        Task에서 이 함수를 나중에 호출해주지 않으면
        비헤이비어 시스템은 해당 Task에 계속 머물게됨.*/
}